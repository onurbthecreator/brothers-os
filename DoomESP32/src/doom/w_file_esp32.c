// Leitura do doom1.wad direto da particao "wad" da flash (nao tem sistema
// de arquivos). O WAD e gravado cru nessa particao pelo esptool, ver
// README.md da pasta doom-esp32.

#include <string.h>

#include "esp_partition.h"

#include "w_file.h"
#include "z_zone.h"

typedef struct
{
    wad_file_t wad;
    const esp_partition_t *particao;
} esp32_wad_file_t;

extern wad_file_class_t esp32_wad_file;

static uint32_t LerU32(const uint8_t *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24);
}

static wad_file_t *W_ESP32_OpenFile(char *path)
{
    const esp_partition_t *p;
    uint8_t cabecalho[12];
    uint32_t tamanho;
    esp32_wad_file_t *result;

    (void)path;

    p = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "wad");
    if (p == NULL || esp_partition_read(p, 0, cabecalho, sizeof(cabecalho)) != ESP_OK)
    {
        return NULL;
    }

    // "IWAD" + numero de lumps + onde fica a tabela: o tamanho do arquivo
    // e o fim da tabela (16 bytes por lump).
    if (memcmp(cabecalho, "IWAD", 4) != 0)
    {
        return NULL;
    }
    tamanho = LerU32(cabecalho + 8) + LerU32(cabecalho + 4) * 16;
    if (tamanho > p->size)
    {
        return NULL;
    }

    result = Z_Malloc(sizeof(esp32_wad_file_t), PU_STATIC, 0);
    result->wad.file_class = &esp32_wad_file;
    result->wad.mapped = NULL;
    result->wad.length = tamanho;
    result->particao = p;

    return &result->wad;
}

static void W_ESP32_CloseFile(wad_file_t *wad)
{
    Z_Free(wad);
}

static size_t W_ESP32_Read(wad_file_t *wad, unsigned int offset,
                           void *buffer, size_t buffer_len)
{
    esp32_wad_file_t *f = (esp32_wad_file_t *)wad;

    if (offset >= wad->length)
    {
        return 0;
    }
    if (offset + buffer_len > wad->length)
    {
        buffer_len = wad->length - offset;
    }
    if (esp_partition_read(f->particao, offset, buffer, buffer_len) != ESP_OK)
    {
        return 0;
    }
    return buffer_len;
}

wad_file_class_t esp32_wad_file =
{
    W_ESP32_OpenFile,
    W_ESP32_CloseFile,
    W_ESP32_Read,
};
