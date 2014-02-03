#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

unsigned char decode_5bit(unsigned char encoded) {
    unsigned char map[] = { '0','1','2','3','4','5','6','7','8','9','B','C','D','F','G','H','J','K','L','M','N','P','Q','R','S','T','V','W','X','Y','Z' };
    return map[(int)encoded];
}

unsigned char decode_6bit(unsigned char encoded) {
    if (encoded & 0x40)
        return (encoded & 0x1F) + 'A' - 1;
    else
        return (encoded & 0x0F) + '0';
}

unsigned char encode_5bit(unsigned char c) {
    unsigned char map[] = { 0xFF, 0x0A, 0x0B, 0x0C, 0xFF, 0x0D, 0x0E, 0x0F, 0xFF, 0x10, 0x11, 0x12, 0x13, 0x14, 0xFF, /* A - O */
                   0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E }; /* P - Z */
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'A' && c <= 'Z')
        return map[c - 'A'];
    else
        return 0xFF;
}

unsigned char encode_6bit(unsigned char c) {
    if (c >= '0' && c <= '9')
        return (c - '0') | 0x30;
    else if (c >= 'A' && c <= 'Z')
        return (c - 'A' + 1) | 0x40;
    else
        return 0xFF;
}

void invalid_tag(void) {
    fprintf(stderr, "Invalid service tag!\n");
    fprintf(stderr, "Service tag must be exactly 7 characters\n");
    fprintf(stderr, "First character can be number [0-9] or character from alphabet [A-Z]\n");
    fprintf(stderr, "Other characters can be numbers [0-9] or characters from alphabet except A,E,I,O,U\n");
    exit(2);
}

int main(int argc, char *argv[]) {
    char service_tag[8] = "       ";
    char encoded_tag[6];

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "CMOS service tag utility for DELL machines v1.0\n");
        fprintf(stderr, "Copyright (c) 2008 Ondrej Zary\n\n");
        fprintf(stderr, "Usage: read tag:  dell_service_tag CMOS_FILE\n");
        fprintf(stderr, "   or write tag:  dell_service_tag CMOS_FILE NEWTAG\n");
        return 1;
    }

    FILE *f = fopen(argv[1], (argc < 3) ? "r" : "r+");
    if (!f) {
        perror("Unable to open CMOS file\n");
        return 1;
    }
    if (fseek(f, 0x59 , SEEK_SET)) {
        perror("Unable to seek to offset 0x59\n");
        return 1;
    }
    if (fread(encoded_tag, 1, sizeof(encoded_tag), f) != sizeof(encoded_tag)) {
        perror("Unable to read tag from CMOS file\n");
        return 1;
    }

    if (argc < 3) { 
        service_tag[0] = decode_6bit(encoded_tag[0] & 0x7F);
        service_tag[1] = decode_5bit((encoded_tag[1] & 0x3E) >> 1);
        service_tag[2] = decode_5bit(((encoded_tag[2] & 0xF0) >> 4) |
                                     (encoded_tag[1] & 0x01) << 4);
        service_tag[3] = decode_5bit(((encoded_tag[2] & 0x0F) << 1) | 
                                     ((encoded_tag[3] & 0x80) >> 7));
        service_tag[4] = decode_5bit((encoded_tag[3] & 0x7C) >> 2);
        service_tag[5] = decode_5bit(((encoded_tag[3] & 0x03) << 3) |
                                     ((encoded_tag[4] & 0xE0) >> 5));
        service_tag[6] = decode_5bit(encoded_tag[4] & 0x1F);
        printf("%s\n", service_tag);
    }

    if (argc > 2) {
        unsigned char c;

        if (strlen(argv[2]) != 7)
            invalid_tag();

        memset(&encoded_tag[1], 0, sizeof(encoded_tag) - 1);
        encoded_tag[0] &= 0x80;

        c = encode_6bit(toupper(argv[2][0]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[0] = c | 0x80;
        c = encode_5bit(toupper(argv[2][1]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[1] |= c << 1;
        c = encode_5bit(toupper(argv[2][2]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[1] |= (c & 0x10) >> 4;
        encoded_tag[2] |= (c & 0x0F) << 4;
        c = encode_5bit(toupper(argv[2][3]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[2] |= (c & 0x1E) >> 1;
        encoded_tag[3] |= (c & 0x01) << 7;
        c = encode_5bit(toupper(argv[2][4]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[3] |= c << 2;
        c = encode_5bit(toupper(argv[2][5]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[3] |= (c & 0x18) >> 3;
        encoded_tag[4] |= (c & 0x07) << 5;
        c = encode_5bit(toupper(argv[2][6]));
        if (c == 0xFF)
            invalid_tag();
        encoded_tag[4] |= c;

        /* Calculate checksum */
        for (unsigned int i = 0; i < sizeof(encoded_tag) - 1; i++)
            encoded_tag[sizeof(encoded_tag) - 1] += encoded_tag[i];
        if (fseek(f, 0x59 , SEEK_SET)) {
            perror("Unable to seek to offset 0x59\n");
            return 1;
        }
        if (fwrite(encoded_tag, 1, sizeof(encoded_tag), f) != sizeof(encoded_tag)) {
            perror("Unable to write tag to CMOS file\n");
            return 1;
        }
    }

    if (fclose(f)) {
        perror("Error closing CMOS file\n");
        return 1;
    }
    return 0;
}
