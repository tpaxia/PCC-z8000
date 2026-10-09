/*
 * run_emu.cpp -- Z8002 emulator test driver for PCC s.out binaries
 *
 * Loads a combined NONSEG s.out executable, sets up the Z8002 emulator, runs the
 * program, and reports the return value (R0 after HALT).
 *
 * Usage: run_emu [-t] [-e <expected>] <file.sout>
 *   -t            enable instruction tracing
 *   -e <expected> check R0 against expected value (decimal)
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <z8000/z8000.h>
#include "memory.h"

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t(p[0]) << 8) | p[1];
}

static uint32_t read_be32(const uint8_t *p)
{
    return (uint32_t(read_be16(p)) << 16) | read_be16(p + 2);
}

struct sout_hdr {
    uint32_t tsize, dsize, bsize, entry;
};

/* The standalone Z8002 has one flat address space. Split/SEG executable
 * testing belongs to the Unix MMU runner, not this loader. */
static int load_sout(const char *path, sout_hdr *hdr,
                     uint8_t **text_out, uint8_t **data_out)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "run_emu: cannot open %s\n", path);
        return -1;
    }
    uint8_t raw[40];
    if (fread(raw, 1, sizeof(raw), f) != sizeof(raw)) {
        fprintf(stderr, "run_emu: short s.out header in %s\n", path);
        fclose(f);
        return -1;
    }
    hdr->tsize = read_be16(raw + 28);
    hdr->dsize = read_be16(raw + 30);
    hdr->bsize = read_be16(raw + 32);
    hdr->entry = read_be16(raw + 16);
    uint32_t image = hdr->tsize + hdr->dsize;
    uint16_t attrs = read_be16(raw + 34);
    uint16_t symbols = read_be16(raw + 12);
    if (read_be16(raw) != 0xe707 || read_be16(raw + 10) != 16 ||
        read_be16(raw + 14) || read_be16(raw + 18) != 1 ||
        read_be32(raw + 20) || read_be32(raw + 24) || read_be32(raw + 36) ||
        read_be32(raw + 2) != image || read_be32(raw + 6) != hdr->bsize ||
        symbols % 14 || (attrs & ~7) || !(attrs & 1) ||
        (hdr->dsize && !(attrs & 2)) || (hdr->bsize && !(attrs & 4)) ||
        !hdr->tsize || (hdr->tsize & 1) || (hdr->entry & 1) ||
        hdr->entry >= hdr->tsize || image + hdr->bsize > 0xfffe) {
        fprintf(stderr, "run_emu: unsupported or invalid s.out in %s\n", path);
        fclose(f);
        return -1;
    }
    if (fseek(f, 0, SEEK_END) || ftell(f) != long(40 + image + symbols) ||
        fseek(f, 40, SEEK_SET)) {
        fprintf(stderr, "run_emu: invalid s.out length in %s\n", path);
        fclose(f);
        return -1;
    }
    *text_out = static_cast<uint8_t *>(malloc(hdr->tsize));
    *data_out = hdr->dsize ? static_cast<uint8_t *>(malloc(hdr->dsize)) : nullptr;
    if (!*text_out || (hdr->dsize && !*data_out) ||
        fread(*text_out, 1, hdr->tsize, f) != hdr->tsize ||
        (hdr->dsize && fread(*data_out, 1, hdr->dsize, f) != hdr->dsize)) {
        fprintf(stderr, "run_emu: cannot read s.out image in %s\n", path);
        fclose(f);
        return -1;
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
            fprintf(stderr, "usage: run_emu [-t] [-e expected] [-c cycles] <file.sout>\n");
            return 1;
        }
    }

    if (!path) {
        fprintf(stderr, "usage: run_emu [-t] [-e expected] [-c cycles] <file.sout>\n");
        return 1;
    }

    /* Load binary */
    sout_hdr hdr;
    uint8_t *text_data = NULL;
    uint8_t *data_data = NULL;
    if (load_sout(path, &hdr, &text_data, &data_data) < 0)
        return 1;

    if (trace) {
        fprintf(stderr, "s.out: text=%u data=%u bss=%u entry=0x%04X\n",
                hdr.tsize, hdr.dsize, hdr.bsize, hdr.entry);
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

    /* Reset before loading: text at zero replaces the PSAP scratch vector. */
    cpu.reset();

    /* Load sections at their combined-space addresses, independent of entry. */
    if (text_data && hdr.tsize > 0) {
        if (!mem.load(0, text_data, hdr.tsize)) {
            fprintf(stderr, "run_emu: failed to load text at 0x%04X\n", hdr.entry);
            return 1;
        }
    }

    /* Load data segment immediately after text */
    if (data_data && hdr.dsize > 0) {
        uint32_t data_addr = hdr.tsize;
        if (!mem.load(data_addr, data_data, hdr.dsize)) {
            fprintf(stderr, "run_emu: failed to load data at 0x%04X\n", data_addr);
            return 1;
        }
    }

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
