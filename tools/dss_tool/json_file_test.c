#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

int main() {
    // Read the JSON file
    FILE *fp = fopen("dss_config_clean.json", "r");
    if (!fp) {
        printf("Error: Cannot open JSON file\n");
        return 1;
    }

    // Get file size
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Read file content
    char *json_content = malloc(file_size + 1);
    if (!json_content) {
        printf("Error: Memory allocation failed\n");
        fclose(fp);
        return 1;
    }

    size_t result = fread(json_content, 1, file_size, fp);
    (void)result; // Silence unused result warning
    json_content[file_size] = '\0';
    fclose(fp);

    printf("JSON content loaded, size: %ld bytes\n", file_size);
    
    // Parse JSON
    cJSON *json = cJSON_Parse(json_content);
    if (!json) {
        printf("Error: Failed to parse JSON\n");
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            printf("Error before: %s\n", error_ptr);
            // Calculate error position
            int pos = error_ptr - json_content;
            printf("Error position: %d\n", pos);
            
            // Print context around error
            int start = (pos > 20) ? pos - 20 : 0;
            int end = (pos + 20 < file_size) ? pos + 20 : file_size;
            printf("Context: ");
            for (int i = start; i < end; i++) {
                if (i == pos) {
                    printf("[ERROR]");
                }
                printf("%c", json_content[i]);
            }
            printf("\n");
        }
        free(json_content);
        return 1;
    }

    printf("JSON parsed successfully!\n");
    
    // Clean up
    cJSON_Delete(json);
    free(json_content);
    
    return 0;
}
