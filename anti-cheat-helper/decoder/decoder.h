#pragma once
#include <cstdint>
#include <string>
#include <vector>

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>
#include <cstdio>

namespace decoder {

    enum class OperandType : uint8_t {
        REG,
        IMM,
        MEM
    };

    static constexpr const char* reg8[16] = {
        "al","cl","dl","bl","spl","bpl","sil","dil",
        "r8b","r9b","r10b","r11b","r12b","r13b","r14b","r15b"
    };
    static constexpr const char* reg16[16] = {
        "ax","cx","dx","bx","sp","bp","si","di",
        "r8w","r9w","r10w","r11w","r12w","r13w","r14w","r15w"
    };
    static constexpr const char* reg32[16] = {
        "eax","ecx","edx","ebx","esp","ebp","esi","edi",
        "r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"
    };
    static constexpr const char* reg64[16] = {
        "rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
        "r8","r9","r10","r11","r12","r13","r14","r15"
    };

    struct Operand {
        OperandType type;
        uint8_t size;      // bytes
        uint64_t immediate; // immediate or displacement
        std::string reg;   // register name or mem string

        Operand() : type(OperandType::IMM), size(0), immediate(0), reg("") {}
        Operand(OperandType t, uint8_t s, uint64_t imm = 0, std::string r = "")
            : type(t), size(s), immediate(imm), reg(std::move(r)) {}
    };

    struct ModRM {
        uint8_t mod;
        uint8_t reg;
        uint8_t rm;
    };

    struct Instruction {
        const uint8_t* address = nullptr;
        std::string mnemonic;
        size_t length = 0;
        uint8_t opcode = 0;

        bool has_modrm = false;
        bool has_sib = false;
        ModRM modrm{};
        uint8_t sib = 0;

        uint8_t rex = 0;   // rex prefix full byte

        uint64_t displacement = 0;
        uint8_t displacement_size = 0;

        uint64_t immediate = 0;
        uint8_t immediate_size = 0;

        // Prefix flags
        bool prefix_lock = false;
        bool prefix_repne = false;
        bool prefix_rep = false;
        bool prefix_operand_size_override = false;
        bool prefix_address_size_override = false;

        std::vector<Operand> operands;

        Instruction() = default;
    };

    class Decoder {
    public:
        // Decode one instruction, returns true if successful
        // code = pointer to bytes to decode
        // max_len = max bytes available in buffer
        // inst = output decoded instruction data
        bool decode(const uint8_t* code, size_t max_len, Instruction& inst);

        // Helper: Format decoded instruction to Intel-like assembly string
        std::string format_instruction(const Instruction& inst) const;

    private:
        bool is_prefix(uint8_t b) const;
        ModRM decode_modrm(uint8_t b) const;
        void decode_sib(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst) const;
        bool decode_displacement(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst, uint8_t size);
        bool decode_immediate(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst, uint8_t size);
        std::string get_reg_name(uint8_t reg, uint8_t rex, uint8_t size, bool is_modrm_reg = false) const;
        std::string format_mem_operand(const Instruction& inst) const;
        bool decode_two_byte_opcode(const uint8_t* code, size_t max_len, size_t& offset, Instruction& inst, uint8_t opcode);
    };

} // namespace decoder
