#include <stdint.h>
#include <stdio.h>
#include "ssdv.h"
#include "ssdvutils.h"

int main(int argc, char* argv[]) {
	FILE *fin = stdin;
	FILE *fout = stdout;
	char type = SSDV_TYPE_NORMAL;
	int verbose = 1;
	char callsign[7];
	uint8_t image_id = 0;
	int8_t quality = 4;
	int pkt_length = SSDV_PKT_SIZE;
	ssdv_t ssdv;
    FILE *ssdvFile, *jpegFile;


    char rb[] = "rb";
    char wb[] = "wb";
    char encode = argc > 1;

    jpegFile = fopen("rpicam.jpg", encode? rb : wb); 
    ssdvFile = fopen("rpicam.ssdv", encode? wb : rb);	
    fin = encode? jpegFile : ssdvFile;
    fout = (!encode)? jpegFile : ssdvFile;

    ssdv_mem_arena_t data = {0};

    while(1) {
        ssdv_dec_init(&ssdv, pkt_length);

    }
	switch(encode)
	{
	case 0: /* Decode */
		
		if(ssdv_dec_init(&ssdv, pkt_length) != SSDV_OK)
		{
			return(-1);
		}
        ssdv_mem_arena_t decoded;
        data = ssdv_read_file(fin);
        decoded = ssdv_dec_buf(&data, &ssdv);
        fwrite(decoded.buf, decoded.used, 1, fout);
        //ssdv_dec_file(fin, fout, &ssdv);
		break;
	
	case 1: /* Encode */
		
		if(ssdv_enc_init(&ssdv, type, callsign, image_id, quality, pkt_length) != SSDV_OK)
		{
			return(-1);
		}
        ssdv_mem_arena_t encoded;
        ssdv_mem_arena_t data2 = {0};
        data = ssdv_read_file(fin);
        data2.buf = malloc(data.length);
        data2.length = data.length;
        ssdv_memcpy_packet(&data, &data2, 0, data.used);
        encoded = ssdv_enc_buf(&data, &ssdv);

        for (size_t i = 0; i < data.used; i++) {
            printf("%hhu, %hhu\n", data.buf[i], data2.buf[i]);
            if (data.buf[i] != data2.buf[i]) {
                printf("NOT EQUAL!\n");
                break;
            }
        }
        printf("Pointers :%p, %p\n", data.buf, data2.buf);
        fwrite(encoded.buf, encoded.used, 1, fout);
        //ssdv_enc_file(fin, fout, &ssdv);		
        break;
    }
    return 0;
}

