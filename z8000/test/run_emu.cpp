/*
 * run_emu.cpp -- Z8002 emulator test driver for PCC b.out binaries
 *
 * Loads a b.out executable, sets up the Z8002 emulator, runs the
 * program, and reports the return value (R0 after HALT).
 *
 * Usage: run_emu [-t] [-e <expected>] <file.bout>
 *   -t            enable instruction tracing
 *   -e <expected> check R0 against expected value (decimal)
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include "z8000.h"
#include "memory.h"

/* Read a big-endian 32-bit integer from a byte buffer */
static uint32_t read_be32(const uint8_t *p)
{
    return ((uint32_t)p[0] << 24) |
           ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] <<  8) |
           ((uint32_t)p[3]);
}

/* b.out header: 8 big-endian 32-bit fields = 32 bytes */
struct bout_hdr {
    uint32_t fmagic;
    uint32_t tsize;
    uint32_t dsize;
    uint32_t bsize;
    uint32_t ssize;
    uint32_t rtsize;
    uint32_t rdsize;
    uint32_t entry;
};

static int load_bout(const char *path, bout_hdr *hdr,
                     uint8_t **text_out, uint8_t **data_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "run_emu: cannot open %s\n", path);
        return -1;
    }

    /* Current a.out uses 8 big-endian 16-bit fields. Older b.out images
     * use 8 big-endian 32-bit fields; retain support for existing fixtures. */
    uint8_t raw[32];
    if (fread(raw, 1, 16, f) != 16) {
        fprintf(stderr, "run_emu: short header in %s\n", path);
        fclose(f);
        return -1;
    }

    uint16_t magic16 = ((uint16_t)raw[0] << 8) | raw[1];
    if (magic16 == 0407 || magic16 == 0405 ||
        magic16 == 0410 || magic16 == 0411) {
        uint16_t fields[8];
        for (int i = 0; i < 8; ++i)
            fields[i] = ((uint16_t)raw[2*i] << 8) | raw[2*i+1];
        hdr->fmagic = fields[0];
        hdr->tsize = fields[1];
        hdr->dsize = fields[2];
        hdr->bsize = fields[3];
        hdr->ssize = fields[4];
        hdr->entry = fields[5];
        hdr->rtsize = fields[6];
        hdr->rdsize = fields[7];
    } else {
        if (fread(raw + 16, 1, 16, f) != 16) {
            fprintf(stderr, "run_emu: short header in %s\n", path);
            fclose(f);
            return -1;
        }
        hdr->fmagic = read_be32(&raw[0]);
        hdr->tsize  = read_be32(&raw[4]);
        hdr->dsize  = read_be32(&raw[8]);
        hdr->bsize  = read_be32(&raw[12]);
        hdr->ssize  = read_be32(&raw[16]);
        hdr->rtsize = read_be32(&raw[20]);
        hdr->rdsize = read_be32(&raw[24]);
        hdr->entry  = read_be32(&raw[28]);
    }

    /* Validate magic */
    if (hdr->fmagic != 0407 && hdr->fmagic != 0405 &&
        hdr->fmagic != 0410 && hdr->fmagic != 0411) {
        fprintf(stderr, "run_emu: bad magic 0%o in %s\n", hdr->fmagic, path);
        fclose(f);
        return -1;
    }

    /* Read text segment */
    *text_out = NULL;
    *data_out = NULL;
    if (hdr->tsize > 0) {
        *text_out = (uint8_t *)malloc(hdr->tsize);
        if (fread(*text_out, 1, hdr->tsize, f) != hdr->tsize) {
            fprintf(stderr, "run_emu: short text in %s\n", path);
            fclose(f);
            return -1;
        }
    }

    /* Read data segment */
    if (hdr->dsize > 0) {
        *data_out = (uint8_t *)malloc(hdr->dsize);
        if (fread(*data_out, 1, hdr->dsize, f) != hdr->dsize) {
            fprintf(stderr, "run_emu: short data in %s\n", path);
            fclose(f);
            return -1;
        }
    }

    fclose(f);
    return 0;
}

int main(int argc, char **argv)
{
    bool trace = false;
    int expected = -1;
    int cycle_limit = 1000000;
    bool check_expected = false;
    const char *path = NULL;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            trace = true;
        } else if (strcmp(argv[i], "-e") == 0 && i + 1 < argc) {
            expected = atoi(argv[++i]);
            check_expected = true;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            char *end;
            long n = strtol(argv[++i], &end, 10);
            if (*end || n <= 0 || n > 100000000) {
                fprintf(stderr, "run_emu: invalid cycle limit\n");
                return 1;
            }
            cycle_limit = (int)n;
        } else if (argv[i][0] != '-') {
            path = argv[i];
        } else {
            fprintf(stderr, "usage: run_emu [-t] [-e expected] [-c cycles] <file.bout>\n");
            return 1;
        }
    }

    if (!path) {
        fprintf(stderr, "usage: run_emu [-t] [-e expected] [-c cycles] <file.bout>\n");
        return 1;
    }

    /* Load binary */
    bout_hdr hdr;
    uint8_t *text_data = NULL;
    uint8_t *data_data = NULL;
    if (load_bout(path, &hdr, &text_data, &data_data) < 0)
        return 1;

    if (trace) {
        fprintf(stderr, "b.out: magic=0%o text=%u data=%u bss=%u entry=0x%04X\n",
                hdr.fmagic, hdr.tsize, hdr.dsize, hdr.bsize, hdr.entry);
    }

    /* Set up emulator */
    MemoryRegion mem(0x10000);
    IOPorts io;
    z8002_device cpu;

    cpu.set_memory(&mem);
    cpu.set_io(&io);
    if (trace) {
        cpu.set_trace(true);
    }

    /* Write Z8002 PSAP reset vector at address 0x0000:
     *   [0x0000] = 0x0000  (reserved)
     *   [0x0002] = 0x4000  (FCW: system mode)
     *   [0x0004] = entry   (PC)
     */
    mem.write_word(0x0000, 0x0000);
    mem.write_word(0x0002, 0x4000);
    mem.write_word(0x0004, (uint16_t)(hdr.entry & 0xFFFF));

    /* Load text segment at entry address */
    if (text_data && hdr.tsize > 0) {
        if (!mem.load(hdr.entry, text_data, hdr.tsize)) {
            fprintf(stderr, "run_emu: failed to load text at 0x%04X\n", hdr.entry);
            return 1;
        }
    }

    /* Load data segment immediately after text */
    if (data_data && hdr.dsize > 0) {
        uint32_t data_addr = hdr.entry + hdr.tsize;
        if (!mem.load(data_addr, data_data, hdr.dsize)) {
            fprintf(stderr, "run_emu: failed to load data at 0x%04X\n", data_addr);
            return 1;
        }
    }

    /* Reset CPU -- reads FCW and PC from PSAP at address 0 */
    cpu.reset();

    /* Set stack pointer (R15) -- crt0 assumes SP is already valid */
    cpu.set_reg(15, 0xFFFE);

    /* Run with cycle limit */
    cpu.run(cycle_limit);

    if (!cpu.is_halted()) {
        fprintf(stderr, "run_emu: %s: CPU did not halt (cycle limit exceeded)\n", path);
        return 1;
    }

    /* Read return value from R0 */
    uint16_t r0 = cpu.get_reg(0);
    int result = (int)(int16_t)r0;  /* sign-extend to int */

    if (check_expected) {
        if (result == expected) {
            printf("PASS %s (R0=%d)\n", path, result);
            free(text_data);
            free(data_data);
            return 0;
        } else {
            printf("FAIL %s (R0=%d, expected %d)\n", path, result, expected);
            free(text_data);
            free(data_data);
            return 1;
        }
    } else {
        printf("%s: R0=%d (0x%04X)\n", path, result, r0);
    }

    free(text_data);
    free(data_data);
    return 0;
}
