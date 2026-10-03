#include "Vac3/vac3_emulation.h"
#include "Vac3/hook_detector.h"
#include "utils/module_utils.h"
#include "gui/gui.h"

int main() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    
    gui::run(GetModuleHandle(nullptr));
    return 0;
}
