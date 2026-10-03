#include "decoder.h"
#include <cstring> // for memcpy
#include <cstdint>
#include <string>
#include <vector>
#include <cstdio>
#include <cassert>
#include <limits>
#include <sstream>
#include <iomanip>

namespace decoder {

    // --------------------------- Implementation ----------------------------

    bool Decoder::is_prefix(uint8_t b) const {
        // Legacy prefixes + REX prefixes + operand size override + address size override
        return (b == 0xF0 || b == 0xF2 || b == 0xF3 ||     // LOCK, REPNE/REPNZ, REP/REPE/REPZ
            b == 0x2E || b == 0x36 || b == 0x3E ||     // CS, SS, DS segment override
            b == 0x26 || b == 0x64 || b == 0x65 ||     // ES, FS, GS segment override
            b == 0x66 ||                               // Operand size override
            b == 0x67 ||                               // Address size override
            (b >= 0x40 && b <= 0x4F));                 // REX prefixes
    }

    ModRM Decoder::decode_modrm(uint8_t b) const {
        return ModRM{
            static_cast<uint8_t>((b >> 6) & 0x3),
            static_cast<uint8_t>((b >> 3) & 0x7),
            static_cast<uint8_t>(b & 0x7)
        };
    }

    void Decoder::decode_sib(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst) const {
        if (offset < max_len) {
            inst.sib = code[offset++];
            inst.has_sib = true;
        }
    }

    bool Decoder::decode_displacement(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst, uint8_t size) {
        if (offset + size > max_len) return false;
        uint64_t disp = 0;
        memcpy(&disp, code + offset, size);
        offset += size;
        inst.displacement = disp;
        inst.displacement_size = size;
        return true;
    }

    bool Decoder::decode_immediate(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst, uint8_t size) {
        if (offset + size > max_len) return false;
        uint64_t imm = 0;
        memcpy(&imm, code + offset, size);
        offset += size;
        inst.immediate = imm;
        inst.immediate_size = size;
        return true;
    }

    std::string Decoder::get_reg_name(uint8_t reg, uint8_t rex, uint8_t size, bool is_modrm_reg) const {
        // REX bits
        // bit 3 = W (operand size 64-bit)
        // bit 2 = R (reg field extension)
        // bit 1 = X (index field extension)
        // bit 0 = B (base field extension)

        uint8_t ext = 0;
        if (is_modrm_reg) {
            ext = (rex & 0x04) ? 8 : 0; // R bit extends reg field
        }
        else {
            ext = (rex & 0x01) ? 8 : 0; // B bit extends rm/base/index
        }

        uint8_t reg_full = reg + ext;
        if (reg_full >= 16) return "unkreg";

        switch (size) {
        case 1: return reg8[reg_full];
        case 2: return reg16[reg_full];
        case 4: return reg32[reg_full];
        case 8:
        default:
            return reg64[reg_full];
        }
    }

    std::string Decoder::format_mem_operand(const Instruction& inst) const {
        // Decode memory operand fully with ModRM, SIB, displacement, RIP-relative

        if (inst.modrm.mod == 0 && inst.modrm.rm == 5) {
            // RIP-relative addressing
            char buf[64];
            snprintf(buf, sizeof(buf), "rip+0x%llx", (unsigned long long)inst.displacement);
            return std::string("[") + buf + "]";
        }

        std::string result = "[";

        uint8_t mod = inst.modrm.mod;
        uint8_t rm = inst.modrm.rm;
        uint8_t rex = inst.rex;

        if (rm == 4 && inst.has_sib) {
            uint8_t scale = (inst.sib >> 6) & 0x3;
            uint8_t index = (inst.sib >> 3) & 0x7;
            uint8_t base = inst.sib & 0x7;

            bool need_plus = false;

            if (!(mod == 0 && base == 5)) {
                result += get_reg_name(base, rex, 8, false);
                need_plus = true;
            }

            if (index != 4) {
                if (need_plus) result += "+";
                std::string idx_reg = get_reg_name(index, rex, 8, false);
                if (scale > 0) {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "%s*%d", idx_reg.c_str(), 1 << scale);
                    result += buf;
                }
                else {
                    result += idx_reg;
                }
                need_plus = true;
            }

            if (inst.displacement_size > 0) {
                char buf[64];
                snprintf(buf, sizeof(buf), "+0x%llx", (unsigned long long)inst.displacement);
                result += buf;
            }
            result += "]";
            return result;
        }
        else {
            // No SIB
            result += get_reg_name(rm, rex, 8, false);

            if (inst.displacement_size > 0) {
                char buf[64];
                snprintf(buf, sizeof(buf), "+0x%llx", (unsigned long long)inst.displacement);
                result += buf;
            }
            result += "]";
            return result;
        }
    }

    bool Decoder::decode(const uint8_t* code, size_t max_len, Instruction& inst) {
        if (!code || max_len == 0) return false;

        size_t offset = 0;
        inst = Instruction{};
        inst.address = code;

        // Parse prefixes
        bool operand_size_override = false;
        bool address_size_override = false;
        while (offset < max_len && is_prefix(code[offset])) {
            uint8_t p = code[offset];
            if (p == 0x66) operand_size_override = true;
            else if (p == 0x67) address_size_override = true;
            else if (p >= 0x40 && p <= 0x4F) inst.rex = p;
            offset++;
        }

        if (offset >= max_len) return false;

        uint8_t opcode = code[offset++];

        // Two-byte opcode
        bool two_byte = false;
        if (opcode == 0x0F) {
            if (offset >= max_len) return false;
            opcode = code[offset++];
            two_byte = true;
        }

        inst.opcode = opcode;

        // Single-byte simple instructions
        switch (opcode) {
        case 0x90: inst.mnemonic = "NOP"; inst.length = offset; return true;
        case 0xC3: inst.mnemonic = "RET"; inst.length = offset; return true;
        case 0xCC: inst.mnemonic = "INT3"; inst.length = offset; return true;
        case 0xC9: inst.mnemonic = "LEAVE"; inst.length = offset; return true;
        }

        if (!two_byte && (opcode == 0x6A || opcode == 0x68)) {
            uint8_t imm_size = (opcode == 0x6A) ? 1 : 4;
            if (offset + imm_size > max_len) return false;
            if (!decode_immediate(code, max_len, offset, inst, imm_size)) return false;
            inst.mnemonic = "PUSH";
            inst.operands.emplace_back(OperandType::IMM, imm_size, inst.immediate);
            inst.length = offset;
            return true;
        }

        // CALL/JMP near relative (E8, E9)
        if (!two_byte && (opcode == 0xE8 || opcode == 0xE9)) {
            if (offset + 4 > max_len) return false;
            int32_t rel;
            memcpy(&rel, code + offset, 4);
            offset += 4;
            inst.mnemonic = (opcode == 0xE8) ? "CALL" : "JMP";
            inst.immediate = static_cast<uint64_t>(rel);
            inst.immediate_size = 4;
            inst.length = offset;
            inst.operands.emplace_back(OperandType::IMM, 4, inst.immediate);
            return true;
        }

        // Short JMP (EB xx)
        if (!two_byte && opcode == 0xEB) {
            if (offset >= max_len) return false;
            int8_t rel8 = static_cast<int8_t>(code[offset++]);
            inst.mnemonic = "JMP";
            inst.immediate = static_cast<uint64_t>(rel8);
            inst.immediate_size = 1;
            inst.length = offset;
            inst.operands.emplace_back(OperandType::IMM, 1, inst.immediate);
            return true;
        }

        // Conditional jumps (Jcc)
        if ((!two_byte && opcode >= 0x70 && opcode <= 0x7F) ||
            (two_byte && opcode >= 0x80 && opcode <= 0x8F)) {
            int size = two_byte ? 4 : 1;
            if (offset + size > max_len) return false;

            int32_t rel = two_byte ? *reinterpret_cast<const int32_t*>(code + offset) : static_cast<int8_t>(code[offset]);
            offset += size;
            inst.mnemonic = "Jcc";
            inst.immediate = static_cast<uint64_t>(rel);
            inst.immediate_size = size;
            inst.length = offset;
            inst.operands.emplace_back(OperandType::IMM, size, inst.immediate);
            return true;
        }

        // PUSH reg (50+rd) and POP reg (58+rd)
        if (!two_byte && (opcode >= 0x50 && opcode <= 0x57)) {
            uint8_t reg = opcode - 0x50;
            if (inst.rex & 0x01) reg |= 8; // REX.B
            inst.mnemonic = "PUSH";
            inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg, inst.rex, 8, true));
            inst.length = offset;
            return true;
        }

        if (!two_byte && (opcode >= 0x58 && opcode <= 0x5F)) {
            uint8_t reg = opcode - 0x58;
            if (inst.rex & 0x01) reg |= 8; // REX.B
            inst.mnemonic = "POP";
            inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg, inst.rex, 8, true));
            inst.length = offset;
            return true;
        }

        // MOV reg64, imm64 (B8+rd)
        if (!two_byte && opcode >= 0xB8 && opcode <= 0xBF) {
            uint8_t reg = opcode - 0xB8;
            if (inst.rex & 0x01) reg |= 8;
            uint8_t imm_size = (inst.rex & 0x08) ? 8 : 4;
            if (offset + imm_size > max_len) return false;
            if (!decode_immediate(code, max_len, offset, inst, imm_size)) return false;
            inst.mnemonic = "MOV";
            inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg, inst.rex, 8, true));
            inst.operands.emplace_back(OperandType::IMM, imm_size, inst.immediate);
            inst.length = offset;
            return true;
        }

        // MOV r/m64, r64 or vice versa (0x89 / 0x8B)
        if (!two_byte && (opcode == 0x89 || opcode == 0x8B)) {
            if (offset >= max_len) return false;
            uint8_t modrm_byte = code[offset++];
            inst.has_modrm = true;
            inst.modrm = decode_modrm(modrm_byte);

            uint8_t reg_field = inst.modrm.reg;
            uint8_t rm_field = inst.modrm.rm;
            // Apply REX bits
            if (inst.rex) {
                if (inst.rex & 0x04) reg_field |= 8; // REX.R
                if (inst.rex & 0x01) rm_field |= 8;  // REX.B
            }

            if (inst.modrm.mod != 3 && inst.modrm.rm == 4) {
                decode_sib(code, max_len, offset, inst);
            }

            // Displacement decode
            if (inst.modrm.mod == 1) {
                if (!decode_displacement(code, max_len, offset, inst, 1)) return false;
            }
            else if (inst.modrm.mod == 2) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }
            else if (inst.modrm.mod == 0 && inst.modrm.rm == 5) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }

            inst.operands.clear();
            if (opcode == 0x8B) {
                inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg_field, inst.rex, 8, true));
                if (inst.modrm.mod == 3) {
                    inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(rm_field, inst.rex, 8, false));
                }
                else {
                    inst.operands.emplace_back(OperandType::MEM, 8, 0, format_mem_operand(inst));
                }
            }
            else {
                if (inst.modrm.mod == 3) {
                    inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(rm_field, inst.rex, 8, false));
                }
                else {
                    inst.operands.emplace_back(OperandType::MEM, 8, 0, format_mem_operand(inst));
                }
                inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg_field, inst.rex, 8, true));
            }

            inst.length = offset;
            inst.mnemonic = "MOV";
            return true;
        }

        // LEA (0x8D)
        if (!two_byte && opcode == 0x8D) {
            if (offset >= max_len) return false;
            uint8_t modrm_byte = code[offset++];
            inst.has_modrm = true;
            inst.modrm = decode_modrm(modrm_byte);

            uint8_t reg_field = inst.modrm.reg;
            uint8_t rm_field = inst.modrm.rm;

            if (inst.rex) {
                if (inst.rex & 0x04) reg_field |= 8;
                if (inst.rex & 0x01) rm_field |= 8;
            }

            if (inst.modrm.mod != 3 && inst.modrm.rm == 4) {
                decode_sib(code, max_len, offset, inst);
            }

            if (inst.modrm.mod == 1) {
                if (!decode_displacement(code, max_len, offset, inst, 1)) return false;
            }
            else if (inst.modrm.mod == 2) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }
            else if (inst.modrm.mod == 0 && inst.modrm.rm == 5) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }

            inst.operands.clear();
            inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(reg_field, inst.rex, 8, true));
            if (inst.modrm.mod == 3) {
                inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(rm_field, inst.rex, 8, false));
            }
            else {
                inst.operands.emplace_back(OperandType::MEM, 8, 0, format_mem_operand(inst));
            }

            inst.length = offset;
            inst.mnemonic = "LEA";
            return true;
        }

        // Group 1 instructions (ADD/SUB/CMP) with immediate: 0x81/0x83 + ModRM + imm
        if (!two_byte && (opcode == 0x81 || opcode == 0x83)) {
            if (offset >= max_len) return false;
            uint8_t modrm_byte = code[offset++];
            inst.has_modrm = true;
            inst.modrm = decode_modrm(modrm_byte);

            uint8_t reg_field = inst.modrm.reg;
            uint8_t rm_field = inst.modrm.rm;

            if (inst.rex) {
                if (inst.rex & 0x04) reg_field |= 8;
                if (inst.rex & 0x01) rm_field |= 8;
            }

            if (inst.modrm.mod != 3 && inst.modrm.rm == 4) {
                decode_sib(code, max_len, offset, inst);
            }

            if (inst.modrm.mod == 1) {
                if (!decode_displacement(code, max_len, offset, inst, 1)) return false;
            }
            else if (inst.modrm.mod == 2) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }
            else if (inst.modrm.mod == 0 && inst.modrm.rm == 5) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }

            uint8_t imm_size = (opcode == 0x81) ? 4 : 1;
            if (offset + imm_size > max_len) return false;

            uint64_t imm = 0;
            memcpy(&imm, code + offset, imm_size);
            offset += imm_size;

            inst.operands.clear();
            if (inst.modrm.mod == 3) {
                inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(rm_field, inst.rex, 8, false));
            }
            else {
                inst.operands.emplace_back(OperandType::MEM, 8, 0, format_mem_operand(inst));
            }
            inst.operands.emplace_back(OperandType::IMM, imm_size, imm);

            switch (reg_field) {
            case 0: inst.mnemonic = "ADD"; break;
            case 5: inst.mnemonic = "SUB"; break;
            case 7: inst.mnemonic = "CMP"; break;
            default: inst.mnemonic = "UNKNOWN"; break;
            }

            inst.length = offset;
            return true;
        }

        // Opcode 0xFF group (CALL/JMP/PUSH r/m64)
        if (!two_byte && opcode == 0xFF) {
            if (offset >= max_len) return false;
            uint8_t modrm_byte = code[offset++];
            inst.has_modrm = true;
            inst.modrm = decode_modrm(modrm_byte);

            uint8_t reg_field = inst.modrm.reg;
            uint8_t rm_field = inst.modrm.rm;

            if (inst.rex) {
                if (inst.rex & 0x04) reg_field |= 8;
                if (inst.rex & 0x01) rm_field |= 8;
            }

            if (inst.modrm.mod != 3 && inst.modrm.rm == 4) {
                decode_sib(code, max_len, offset, inst);
            }

            if (inst.modrm.mod == 1) {
                if (!decode_displacement(code, max_len, offset, inst, 1)) return false;
            }
            else if (inst.modrm.mod == 2) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }
            else if (inst.modrm.mod == 0 && inst.modrm.rm == 5) {
                if (!decode_displacement(code, max_len, offset, inst, 4)) return false;
            }

            inst.operands.clear();

            // Special case: if mod==0 and rm==5 and reg is CALL(2) or JMP(4)
            if (inst.modrm.mod == 0 && inst.modrm.rm == 5 && (reg_field == 2 || reg_field == 4)) {
                // RIP-relative CALL or JMP
                inst.mnemonic = (reg_field == 2) ? "CALL" : "JMP";
                inst.immediate = inst.displacement;
                inst.immediate_size = inst.displacement_size;
                inst.operands.emplace_back(OperandType::MEM, 8, inst.displacement, "[rip+disp]");
                inst.length = offset;
                return true;
            }

            // Otherwise interpret operand as r/m64
            if (inst.modrm.mod == 3) {
                inst.operands.emplace_back(OperandType::REG, 8, 0, get_reg_name(rm_field, inst.rex, 8, false));
            }
            else {
                inst.operands.emplace_back(OperandType::MEM, 8, inst.displacement, format_mem_operand(inst));
            }

            switch (reg_field) {
            case 0: inst.mnemonic = "INC"; break;
            case 1: inst.mnemonic = "DEC"; break;
            case 2: inst.mnemonic = "CALL"; break;
            case 3: inst.mnemonic = "CALLF"; break;
            case 4: inst.mnemonic = "JMP"; break;
            case 5: inst.mnemonic = "JMPF"; break;
            case 6: inst.mnemonic = "PUSH"; break;
            default: inst.mnemonic = "UNKNOWN"; break;
            }
            inst.length = offset;
            return true;
        }

        // Unknown or unsupported opcode
        return false;
    }

} // namespace decoder
