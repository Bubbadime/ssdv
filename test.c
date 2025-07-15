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

    fprintf(fout, "Pre-open Fin: %p, Fout: %p\n", fin, fout);
    jpegFile = fopen("rpicam.jpg", encode? rb : wb); 
    ssdvFile = fopen("rpicam.ssdv", encode? wb : rb);	
    fin = encode? jpegFile : ssdvFile;
    fout = (!encode)? jpegFile : ssdvFile;
    printf("Fin: %p, Fout: %p\n", fin, fout);

    uint8_t *data = 0;
    size_t dataLen;

	switch(encode)
	{
	case 0: /* Decode */
		
		if(ssdv_dec_init(&ssdv, pkt_length) != SSDV_OK)
		{
			return(-1);
		}

#define MODE_BUF 1
#if MODE_BUF
        uint8_t *decoded;
        size_t decodedLen;
        printf("Read file next\n");
        data = ssdv_read_file(fin, &dataLen);
        decoded = ssdv_dec_buf(data, dataLen, &ssdv, &decodedLen);
        fwrite(decoded, decodedLen, 1, fout);
#else
        ssdv_dec_file(fin, fout, &ssdv);
#endif
		break;
	case 1: /* Encode */
		
		if(ssdv_enc_init(&ssdv, type, callsign, image_id, quality, pkt_length) != SSDV_OK)
		{
			return(-1);
		}
#if MODE_BUF
        uint8_t *encoded;
        size_t encodedLen;
        data = ssdv_read_file(fin, &dataLen);
        encoded = ssdv_enc_buf(data, dataLen, &ssdv, &encodedLen);
        fwrite(encoded, encodedLen, 1, fout);
#else
        ssdv_enc_file(fin, fout, &ssdv);		
#endif
        break;
    }
    return 0;
}

