#include "ark5.h"

// This is taken from ARK5 here: https://github.com/PSP-Arkfive/ARK-5/blob/a9b74547c0d1785cc77981462cf095bd5a76434f/SystemControl/src/sctrl_hen.c#L282
// It does not appear to exist in ARK4 or any other CFW
// Manually applying it so i am not locked into ARK5 but would like to remove when enough people upgrae
u32 _sctrlHENFindImportInMod(SceModule * mod, const char *library, u32 nid) {
    // Invalid Arguments
    if (mod == NULL || library == NULL) {
        return 0;
    }

    for(int i = 0; i < mod->stub_size;) {
        SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

        if(stub->libname && strcmp(stub->libname, library) == 0) {
            unsigned int *table = stub->nidtable;

            for(int j = 0; j < stub->stubcount; j++) {
                if(table[j] == nid) {
                    return ((u32)stub->stubtable + (j * 8));
                }
            }
        }

        i += (stub->len * 4);
    }

    return 0;
}