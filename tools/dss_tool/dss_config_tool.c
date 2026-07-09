#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "cJSON.h"
#include "dss.h"

#define VERSION "1.0.5"

// Function declarations for initialization functions
void init_ak_qs_dss_info(struct ak_qs_dss_info *dss_info);
int load_dss_info_from_bin_with_header(struct ak_qs_dss_info *dss_info, const char *filename);
void init_ak_qs_vi_info(struct ak_qs_vi_info *vi_info);
void init_ak_qs_vi_dev_info(struct ak_qs_vi_dev_info *dev_info);
void init_ak_qs_vi_chn_info(struct ak_qs_vi_chn_info *chn_info);
void init_ak_qs_venc_info(struct ak_qs_venc_info *venc_info);
void init_ak_qs_video_info(struct ak_qs_video_info *video_info);
void init_ak_qs_isp_info(struct ak_qs_isp_info *isp_info);
void init_dss_hw_ircut(struct dss_hw_ircut *ircut);
void init_dss_hw_led(struct dss_hw_led *led);
void init_dss_hw_lumen_sensor(struct dss_hw_lumen_sensor *sensor);
void init_qs_crop_info(struct qs_crop_info *crop);
void init_fastae_info(struct fastae_info *fastae);
void init_dss_gpio(struct dss_gpio *gpio);
void init_dss_pwm(struct dss_pwm *pwm);
void init_dss_stitch_attr(struct dss_stitch_attr *stitch);
void init_dss_npu_attr(struct dss_npu_attr *npu_attr);
void init_ak_qs_audio_info(struct ak_qs_audio_info *audio_info);
void init_ak_qs_ai_info(struct ak_qs_ai_info *ai_info);
void init_ak_qs_ao_info(struct ak_qs_ao_info *ao_info);
void init_ak_qs_npu_info(struct ak_qs_npu_info *npu_info);
void init_ak_little_core_info(struct ak_little_core_info *little_core_info);
void init_ak_isp_3dnr_buf(struct ak_isp_3dnr_buf *buf);

// Function declarations
int validate_json_structure(cJSON *json_obj, const char *struct_name);
int fill_ak_qs_dss_info_from_json(cJSON *json, struct ak_qs_dss_info *dss_info);
int save_dss_info_to_bin(struct ak_qs_dss_info *dss_info, const char *filename);
int load_dss_info_from_bin(struct ak_qs_dss_info *dss_info, const char *filename);
void print_dss_info_as_json(struct ak_qs_dss_info *dss_info);
void print_usage(const char *program_name);
void print_version(void);

// Helper functions for filling structures
int fill_qs_vi_info_from_json(cJSON *json, struct ak_qs_vi_info *vi_info);
int fill_qs_vi_dev_info_from_json(cJSON *json, struct ak_qs_vi_dev_info *dev_info);
int fill_qs_vi_chn_info_from_json(cJSON *json, struct ak_qs_vi_chn_info *chn_info);
int fill_qs_venc_info_from_json(cJSON *json, struct ak_qs_venc_info *venc_info);
int fill_qs_video_info_from_json(cJSON *json, struct ak_qs_video_info *video_info);
int fill_isp_info_from_json(cJSON *json, struct ak_qs_isp_info *isp_info);
int fill_dss_hw_ircut_from_json(cJSON *json, struct dss_hw_ircut *ircut);
int fill_dss_hw_led_from_json(cJSON *json, struct dss_hw_led *led);
int fill_dss_hw_lumen_sensor_from_json(cJSON *json, struct dss_hw_lumen_sensor *sensor);
int fill_crop_info_from_json(cJSON *json, struct qs_crop_info *crop);
int fill_fastae_info_from_json(cJSON *json, struct fastae_info *fastae);
int fill_dss_gpio_from_json(cJSON *json, struct dss_gpio *gpio);
int fill_dss_pwm_from_json(cJSON *json, struct dss_pwm *pwm);
int fill_dss_stitch_attr_from_json(cJSON *json, struct dss_stitch_attr *stitch);
int fill_dss_npu_attr_from_json(cJSON *json, struct dss_npu_attr *npu_attr);
int fill_ak_qs_audio_info_from_json(cJSON *json, struct ak_qs_audio_info *audio_info);
int fill_ak_qs_ai_info_from_json(cJSON *json, struct ak_qs_ai_info *ai_info);
int fill_ak_qs_ao_info_from_json(cJSON *json, struct ak_qs_ao_info *ao_info);
int fill_ak_qs_npu_info_from_json(cJSON *json, struct ak_qs_npu_info *npu_info);
int fill_ak_little_core_info_from_json(cJSON *json, struct ak_little_core_info *little_core_info);

// Helper functions for printing structures
void print_qs_vi_info_as_json(struct ak_qs_vi_info *vi_info);
void print_qs_vi_dev_info_as_json(struct ak_qs_vi_dev_info *dev_info);
void print_qs_vi_chn_info_as_json(struct ak_qs_vi_chn_info *chn_info);
void print_qs_venc_info_as_json(struct ak_qs_venc_info *venc_info);
void print_qs_video_info_as_json(struct ak_qs_video_info *video_info);
void print_isp_info_as_json(struct ak_qs_isp_info *isp_info);
void print_dss_hw_ircut_as_json(struct dss_hw_ircut *ircut);
void print_dss_hw_led_as_json(struct dss_hw_led *led);
void print_dss_hw_lumen_sensor_as_json(struct dss_hw_lumen_sensor *sensor);
void print_crop_info_as_json(struct qs_crop_info *crop);
void print_fastae_info_as_json(struct fastae_info *fastae);
void print_dss_gpio_as_json(struct dss_gpio *gpio);
void print_dss_pwm_as_json(struct dss_pwm *pwm);
void print_dss_stitch_attr_as_json(struct dss_stitch_attr *stitch);
void print_dss_npu_attr_as_json(struct dss_npu_attr *npu_attr);
void print_ak_qs_audio_info_as_json(struct ak_qs_audio_info *audio_info);
void print_ak_qs_ai_info_as_json(struct ak_qs_ai_info *ai_info);
void print_ak_qs_ao_info_as_json(struct ak_qs_ao_info *ao_info);
void print_ak_qs_npu_info_as_json(struct ak_qs_npu_info *npu_info);
void print_ak_little_core_info_as_json(struct ak_little_core_info *little_core_info);

// Implementation of initialization functions
void init_ak_qs_dss_info(struct ak_qs_dss_info *dss_info) {
    // Initialize int fields to -1, others to 0
    memset(dss_info, 0, sizeof(struct ak_qs_dss_info));
    
    // Initialize int fields to -1
    for (int i = 0; i < DSS_VIDEO_MAX_NUM; i++) {
        init_ak_qs_vi_info(&dss_info->qs_vi_info[i]);
    }
    
    for (int i = 0; i < DSS_VENC_MAX_NUM; i++) {
        init_ak_qs_venc_info(&dss_info->qs_venc_info[i]);
    }
    
    for (int i = 0; i < DSS_VENC_MAX_NUM; i++) {
        init_ak_qs_video_info(&dss_info->qs_video_info[i]);
    }
    
    for (int i = 0; i < DSS_ISP_MAX_NUM; i++) {
        init_ak_qs_isp_info(&dss_info->isp_info[i]);
    }
    
    for (int i = 0; i < DSS_IRCUT_MAX_NUM; i++) {
        init_dss_hw_ircut(&dss_info->ircut[i]);
    }
    
    for (int i = 0; i < DSS_LED_MAX_NUM; i++) {
        init_dss_hw_led(&dss_info->led[i]);
    }
    
    for (int i = 0; i < DSS_MAX_AIN_NUM; i++) {
        init_dss_hw_lumen_sensor(&dss_info->hw_lumi_sensor[i]);
    }
    
    init_ak_qs_audio_info(&dss_info->audio_info);
    
    for (int i = 0; i < DSS_NPU_MAX_NUM; i++) {
        init_ak_qs_npu_info(&dss_info->npu_info[i]);
    }
    
    init_ak_little_core_info(&dss_info->little_core_info);
}

void init_ak_qs_vi_info(struct ak_qs_vi_info *vi_info) {
    init_ak_qs_vi_dev_info(&vi_info->qs_vi_dev_info);
    
    for (int i = 0; i < DSS_VIDEO_CHN_NUM; i++) {
        init_ak_qs_vi_chn_info(&vi_info->qs_vi_chn_info[i]);
    }
}

void init_ak_qs_vi_dev_info(struct ak_qs_vi_dev_info *dev_info) {
    // Initialize int fields to -1, others to 0
    memset(dev_info, 0, sizeof(struct ak_qs_vi_dev_info));
    
    // Initialize int fields to -1
    dev_info->dev_id = -1;
    dev_info->enable = -1;
    dev_info->isp_id = -1;
    dev_info->frame_rate = -1;
    dev_info->ircut_group_id = -1;
    dev_info->led_group_id = -1;
    dev_info->lumen_group_id = -1;
    
    // Initialize sub-structures
    init_qs_crop_info(&dev_info->crop);
    init_ak_isp_3dnr_buf(&dev_info->isp_3dnr_info);
    init_fastae_info(&dev_info->fastae_info);
}

void init_ak_qs_vi_chn_info(struct ak_qs_vi_chn_info *chn_info) {
    // Initialize int fields to -1, others to 0
    memset(chn_info, 0, sizeof(struct ak_qs_vi_chn_info));
    
    // Initialize int fields to -1
    chn_info->dev_id = -1;
    chn_info->chn_id = -1;
    chn_info->enable = -1;
    chn_info->frame_rate = -1;
    chn_info->frame_depth = -1;
    chn_info->data_type = -1;
    chn_info->mode = -1;
    
    // Initialize sub-structures
    init_qs_crop_info(&chn_info->crop);
    init_dss_stitch_attr(&chn_info->stitch_attr);
    init_dss_npu_attr(&chn_info->npu_attr);
}

void init_ak_qs_venc_info(struct ak_qs_venc_info *venc_info) {
    // Initialize int fields to -1, others to 0
    memset(venc_info, 0, sizeof(struct ak_qs_venc_info));
    
    // Initialize int fields to -1
    // Note: Most fields in this structure are unsigned int, so they stay 0
    venc_info->venc_id = -1;
}

void init_ak_qs_video_info(struct ak_qs_video_info *video_info) {
    // Initialize int fields to -1, others to 0
    memset(video_info, 0, sizeof(struct ak_qs_video_info));
    
    // Initialize int fields to -1
    video_info->chn_id = -1;
    video_info->venc_id = -1;
    video_info->enable = -1;
    video_info->frame_rate = -1;
    video_info->frame_depth = -1;
    video_info->venc_encode_mode = -1;
}

void init_ak_qs_isp_info(struct ak_qs_isp_info *isp_info) {
    // Initialize int fields to -1, others to 0
    memset(isp_info, 0, sizeof(struct ak_qs_isp_info));
    
    // Initialize int fields to -1
    isp_info->isp_id = -1;
}

void init_dss_hw_ircut(struct dss_hw_ircut *ircut) {
    // Initialize int fields to -1, others to 0
    memset(ircut, 0, sizeof(struct dss_hw_ircut));
    
    // Initialize int fields to -1
    // Note: This structure contains sub-structures, not int fields directly
    init_dss_gpio(&ircut->ircut_a_gpio);
    init_dss_gpio(&ircut->ircut_b_gpio);
}

void init_dss_hw_led(struct dss_hw_led *led) {
    // Initialize int fields to -1, others to 0
    memset(led, 0, sizeof(struct dss_hw_led));
    
    // Initialize int fields to -1
    led->ircut_led_type = -1;
    led->whiteled_type = -1;
    
    // Initialize sub-structures
    init_dss_gpio(&led->irled_gpio);
    init_dss_pwm(&led->irled_pwm);
    init_dss_gpio(&led->whiteled_gpio);
    init_dss_pwm(&led->whiteled_pwm);
}

void init_dss_hw_lumen_sensor(struct dss_hw_lumen_sensor *sensor) {
    // Initialize int fields to -1, others to 0
    memset(sensor, 0, sizeof(struct dss_hw_lumen_sensor));
    
    // Initialize int fields to -1
    sensor->ain_index = -1;
    sensor->daynight_threshold = -1;
}

void init_qs_crop_info(struct qs_crop_info *crop) {
    // Initialize int fields to -1, others to 0
    memset(crop, 0, sizeof(struct qs_crop_info));
    
    // Initialize int fields to -1
    crop->left = -1;
    crop->top = -1;
    crop->width = -1;
    crop->height = -1;
}

void init_fastae_info(struct fastae_info *fastae) {
    // Initialize int fields to -1, others to 0
    memset(fastae, 0, sizeof(struct fastae_info));
    
    // Initialize int fields to -1
    fastae->bining_mode = -1;
    fastae->fastae_mode = -1;
    fastae->ae_stable_range = -1;
    fastae->max_run_frames = -1;
    fastae->night_mode_light_select = -1;
    fastae->daynight_master_id = -1;
    
    // Initialize array elements to -1
    for (int i = 0; i < 2; i++) {
        fastae->daynight_threshold[i] = -1;
    }
}

void init_dss_gpio(struct dss_gpio *gpio) {
    // Initialize int fields to -1, others to 0
    memset(gpio, 0, sizeof(struct dss_gpio));
    
    // Initialize int fields to -1
    gpio->nb = -1;
    gpio->value = -1;
}

void init_dss_pwm(struct dss_pwm *pwm) {
    // Initialize int fields to -1, others to 0
    memset(pwm, 0, sizeof(struct dss_pwm));
    
    // Initialize int fields to -1
    pwm->pwm_id = -1;
    pwm->duty_ns = -1;
    pwm->period_ns = -1;
}

void init_dss_stitch_attr(struct dss_stitch_attr *stitch) {
    // Initialize int fields to -1, others to 0
    memset(stitch, 0, sizeof(struct dss_stitch_attr));
    
    // Initialize int fields to -1
    stitch->stitch_mode = -1;
    stitch->stitch_chn_id = -1;
    stitch->stitch_num = -1;
    stitch->stitch_index = -1;
    stitch->stitch_global_id = -1;
}

void init_dss_npu_attr(struct dss_npu_attr *npu_attr) {
    // Initialize int fields to -1, others to 0
    memset(npu_attr, 0, sizeof(struct dss_npu_attr));
    
    // Initialize int fields to -1
    npu_attr->npu_enable = -1;
    
    // Initialize array elements to -1
    for (int i = 0; i < DSS_NPU_MAX_NUM; i++) {
        npu_attr->npu_chn_id[i] = -1;
    }
}

void init_ak_qs_audio_info(struct ak_qs_audio_info *audio_info) {
    // Initialize int fields to -1, others to 0
    memset(audio_info, 0, sizeof(struct ak_qs_audio_info));
    
    // Initialize int fields to -1
    init_ak_qs_ai_info(&audio_info->ai_info);
    init_ak_qs_ao_info(&audio_info->ao_info);
}

void init_ak_qs_ai_info(struct ak_qs_ai_info *ai_info) {
    // Initialize int fields to -1, others to 0
    memset(ai_info, 0, sizeof(struct ak_qs_ai_info));
    
    // Initialize int fields to -1
    ai_info->id = -1;
    ai_info->dev_type = -1;
    ai_info->gain = -1;
    ai_info->data_type = -1;
}

void init_ak_qs_ao_info(struct ak_qs_ao_info *ao_info) {
    // Initialize int fields to -1, others to 0
    memset(ao_info, 0, sizeof(struct ak_qs_ao_info));
    
    // Initialize int fields to -1
    ao_info->id = -1;
    ao_info->dev_type = -1;
    ao_info->gain = -1;
    ao_info->data_type = -1;
    ao_info->data_src = -1;
}

void init_ak_qs_npu_info(struct ak_qs_npu_info *npu_info) {
    // Initialize int fields to -1, others to 0
    memset(npu_info, 0, sizeof(struct ak_qs_npu_info));
    
    // Initialize int fields to -1
    npu_info->chn_id = -1;
    npu_info->model_type = -1;
}

void init_ak_little_core_info(struct ak_little_core_info *little_core_info) {
    // Initialize int fields to -1, others to 0
    memset(little_core_info, 0, sizeof(struct ak_little_core_info));
    
    // Note: This structure mainly contains char arrays, so no int fields to initialize to -1
}

// Additional helper function for ak_isp_3dnr_buf
void init_ak_isp_3dnr_buf(struct ak_isp_3dnr_buf *buf) {
    // Initialize int fields to -1, others to 0
    memset(buf, 0, sizeof(struct ak_isp_3dnr_buf));
    
    // Initialize int fields to -1
    buf->isp_3dnr_total_len = -1;
    buf->isp_3dnr_y_len = -1;
    buf->isp_3dnr_u_len = -1;
    buf->isp_3dnr_v_len = -1;
}

int main(int argc, char *argv[]) {
    int opt;
    int json_to_bin = 0;
    int bin_to_json = 0;
    int bin_with_header_to_json = 0;  // 新增：带校验头的BIN文件转换为JSON
    char *input_file = NULL;
    char *output_file = NULL;

    // Define long options
    static struct option long_options[] = {
        {"json-to-bin", required_argument, 0, 'j'},
        {"bin-to-json", required_argument, 0, 'b'},
        {"bin-with-header-to-json", required_argument, 0, 'd'},  // 新增：带校验头的BIN文件选项
        {"help",        no_argument,       0, 'h'},
        {"version",     no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };

    // Parse command line options
    while ((opt = getopt_long(argc, argv, "j:b:d:hHv", long_options, NULL)) != -1) {
        switch (opt) {
            case 'j':
                json_to_bin = 1;
                input_file = optarg;
                if (optind < argc && argv[optind][0] != '-') {
                    output_file = argv[optind++];
                }
                break;
            case 'b':
                bin_to_json = 1;
                input_file = optarg;
                break;
            case 'd':  // 新增：处理带校验头的BIN文件
                bin_with_header_to_json = 1;
                input_file = optarg;
                break;
            case 'h':
            case 'H':
                print_usage(argv[0]);
                return 0;
            case 'v':
                print_version();
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    // Check for remaining arguments
    if (optind < argc) {
        fprintf(stderr, "Error: Unexpected argument '%s'\n", argv[optind]);
        print_usage(argv[0]);
        return 1;
    }

    // Validate options
    // 检查是否同时指定了多个互斥选项
    int option_count = (json_to_bin ? 1 : 0) + (bin_to_json ? 1 : 0) + (bin_with_header_to_json ? 1 : 0);
    if (option_count > 1) {
        fprintf(stderr, "Error: Cannot specify multiple conversion options\n");
        print_usage(argv[0]);
        return 1;
    }

    if (option_count == 0) {
        fprintf(stderr, "Error: Must specify a conversion option\n");
        print_usage(argv[0]);
        return 1;
    }

    if (json_to_bin) {
        if (!input_file || !output_file) {
            fprintf(stderr, "Error: Missing input or output file for JSON to BIN conversion\n");
            print_usage(argv[0]);
            return 1;
        }

        // Read JSON file
        FILE *fp = fopen(input_file, "r");
        if (!fp) {
            fprintf(stderr, "Error: Cannot open JSON file %s\n", input_file);
            return 1;
        }

        // Get file size
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        // Read file content
        char *json_content = malloc(file_size + 1);
        if (!json_content) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(fp);
            return 1;
        }

        size_t result = fread(json_content, 1, file_size, fp);
        (void)result; // Silence unused result warning
        json_content[file_size] = '\0';
        fclose(fp);

        // Parse JSON
        cJSON *json = cJSON_Parse(json_content);
        if (!json) {
            fprintf(stderr, "Error: Failed to parse JSON\n");
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr) {
                fprintf(stderr, "Error before: %s\n", error_ptr);
            }
            free(json_content);
            return 1;
        }

        // Validate JSON structure
        if (!validate_json_structure(json, "ak_qs_dss_info")) {
            fprintf(stderr, "Error: JSON structure validation failed\n");
            cJSON_Delete(json);
            free(json_content);
            return 1;
        }

        // Fill ak_qs_dss_info structure
        struct ak_qs_dss_info dss_info;
        init_ak_qs_dss_info(&dss_info);

        if (!fill_ak_qs_dss_info_from_json(json, &dss_info)) {
            fprintf(stderr, "Error: Failed to fill ak_qs_dss_info from JSON\n");
            cJSON_Delete(json);
            free(json_content);
            return 1;
        }

        // Save to BIN file
        if (!save_dss_info_to_bin(&dss_info, output_file)) {
            fprintf(stderr, "Error: Failed to save DSS info to BIN file\n");
            cJSON_Delete(json);
            free(json_content);
            return 1;
        }

        printf("Successfully converted %s to %s\n", input_file, output_file);

        cJSON_Delete(json);
        free(json_content);
    } else if (bin_to_json) {
        if (!input_file) {
            fprintf(stderr, "Error: Missing input file for BIN to JSON conversion\n");
            print_usage(argv[0]);
            return 1;
        }

        struct ak_qs_dss_info dss_info;

        if (!load_dss_info_from_bin(&dss_info, input_file)) {
            fprintf(stderr, "Error: Failed to load DSS info from BIN file\n");
            return 1;
        }

        print_dss_info_as_json(&dss_info);
    } else if (bin_with_header_to_json) {  // 新增：处理带校验头的BIN文件
        if (!input_file) {
            fprintf(stderr, "Error: Missing input file for BIN with header to JSON conversion\n");
            print_usage(argv[0]);
            return 1;
        }

        struct ak_qs_dss_info dss_info;

        if (!load_dss_info_from_bin_with_header(&dss_info, input_file)) {
            fprintf(stderr, "Error: Failed to load DSS info from BIN file with header\n");
            return 1;
        }

        print_dss_info_as_json(&dss_info);
    }

    return 0;
}

void print_usage(const char *program_name) {
    printf("Usage:\n");
    printf("  %s -j|--json-to-bin <input.json> <output.bin>   Convert JSON to BIN\n", program_name);
    printf("  %s -b|--bin-to-json <input.bin>                 Convert BIN to JSON and print to stdout\n", program_name);
    printf("  %s -d|--bin-with-header-to-json <input.bin>     Convert BIN with 64-byte header to JSON and print to stdout\n", program_name);
    printf("  %s -h|-H|--help                                 Print this help message\n", program_name);
    printf("  %s -v|--version                                 Print version information\n", program_name);
}

void print_version(void) {
    printf("DSS Config Tool Version %s\n", VERSION);
}

int validate_json_structure(cJSON *json_obj, const char *struct_name) {
    if (!cJSON_IsObject(json_obj)) {
        fprintf(stderr, "Error: Expected JSON object for %s\n", struct_name);
        return 0;
    }

    // For now, we'll just check that it's a valid JSON object
    // A more comprehensive validation would check all fields
    return 1;
}

int fill_ak_qs_dss_info_from_json(cJSON *json, struct ak_qs_dss_info *dss_info) {
    cJSON *item;
    
    // Process version
    item = cJSON_GetObjectItemCaseSensitive(json, "version");
    if (item && cJSON_IsNumber(item)) {
        dss_info->version = item->valueint;
    }

    // Process qs_vi_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "qs_vi_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_VIDEO_MAX_NUM; i++) {
            cJSON *vi_info_item = cJSON_GetArrayItem(item, i);
            if (vi_info_item) {
                fill_qs_vi_info_from_json(vi_info_item, &dss_info->qs_vi_info[i]);
            }
        }
    }

    // Process qs_venc_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "qs_venc_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_VENC_MAX_NUM; i++) {
            cJSON *venc_info_item = cJSON_GetArrayItem(item, i);
            if (venc_info_item) {
                fill_qs_venc_info_from_json(venc_info_item, &dss_info->qs_venc_info[i]);
            }
        }
    }

    // Process qs_video_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "qs_video_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_VENC_MAX_NUM; i++) {
            cJSON *video_info_item = cJSON_GetArrayItem(item, i);
            if (video_info_item) {
                fill_qs_video_info_from_json(video_info_item, &dss_info->qs_video_info[i]);
            }
        }
    }

    // Process isp_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "isp_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_ISP_MAX_NUM; i++) {
            cJSON *isp_info_item = cJSON_GetArrayItem(item, i);
            if (isp_info_item) {
                fill_isp_info_from_json(isp_info_item, &dss_info->isp_info[i]);
            }
        }
    }

    // Process ircut array
    item = cJSON_GetObjectItemCaseSensitive(json, "ircut");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_IRCUT_MAX_NUM; i++) {
            cJSON *ircut_item = cJSON_GetArrayItem(item, i);
            if (ircut_item) {
                fill_dss_hw_ircut_from_json(ircut_item, &dss_info->ircut[i]);
            }
        }
    }

    // Process led array
    item = cJSON_GetObjectItemCaseSensitive(json, "led");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_LED_MAX_NUM; i++) {
            cJSON *led_item = cJSON_GetArrayItem(item, i);
            if (led_item) {
                fill_dss_hw_led_from_json(led_item, &dss_info->led[i]);
            }
        }
    }

    // Process hw_lumi_sensor array
    item = cJSON_GetObjectItemCaseSensitive(json, "hw_lumi_sensor");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_MAX_AIN_NUM; i++) {
            cJSON *sensor_item = cJSON_GetArrayItem(item, i);
            if (sensor_item) {
                fill_dss_hw_lumen_sensor_from_json(sensor_item, &dss_info->hw_lumi_sensor[i]);
            }
        }
    }

    // Process audio_info
    item = cJSON_GetObjectItemCaseSensitive(json, "audio_info");
    if (item) {
        fill_ak_qs_audio_info_from_json(item, &dss_info->audio_info);
    }

    // Process npu_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "npu_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_NPU_MAX_NUM; i++) {
            cJSON *npu_info_item = cJSON_GetArrayItem(item, i);
            if (npu_info_item) {
                fill_ak_qs_npu_info_from_json(npu_info_item, &dss_info->npu_info[i]);
            }
        }
    }

    // Process little_core_info
    item = cJSON_GetObjectItemCaseSensitive(json, "little_core_info");
    if (item) {
        fill_ak_little_core_info_from_json(item, &dss_info->little_core_info);
    }

    return 1;
}

int save_dss_info_to_bin(struct ak_qs_dss_info *dss_info, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return 0;
    }

    size_t written = fwrite(dss_info, sizeof(struct ak_qs_dss_info), 1, fp);
    fclose(fp);

    if (written != 1) {
        fprintf(stderr, "Error: Failed to write DSS info to file\n");
        return 0;
    }

    return 1;
}

int load_dss_info_from_bin(struct ak_qs_dss_info *dss_info, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s for reading\n", filename);
        return 0;
    }

    size_t read = fread(dss_info, sizeof(struct ak_qs_dss_info), 1, fp);
    fclose(fp);

    if (read != 1) {
        fprintf(stderr, "Error: Failed to read DSS info from file\n");
        return 0;
    }

    return 1;
}

// 新增函数：从带校验头的BIN文件加载DSS信息
int load_dss_info_from_bin_with_header(struct ak_qs_dss_info *dss_info, const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s for reading\n", filename);
        return 0;
    }

    // 跳过64字节的校验头
    if (fseek(fp, 64, SEEK_SET) != 0) {
        fprintf(stderr, "Error: Failed to seek in file %s\n", filename);
        fclose(fp);
        return 0;
    }

    size_t read = fread(dss_info, sizeof(struct ak_qs_dss_info), 1, fp);
    fclose(fp);

    if (read != 1) {
        fprintf(stderr, "Error: Failed to read DSS info from file\n");
        return 0;
    }

    return 1;
}

void print_dss_info_as_json(struct ak_qs_dss_info *dss_info) {
    printf("{\n");
    printf("  \"version\": %u,\n", dss_info->version);
    printf("  \"qs_vi_info\": [\n");
    for (int i = 0; i < DSS_VIDEO_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_qs_vi_info_as_json(&dss_info->qs_vi_info[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"qs_venc_info\": [\n");
    for (int i = 0; i < DSS_VENC_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_qs_venc_info_as_json(&dss_info->qs_venc_info[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"qs_video_info\": [\n");
    for (int i = 0; i < DSS_VENC_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_qs_video_info_as_json(&dss_info->qs_video_info[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"isp_info\": [\n");
    for (int i = 0; i < DSS_ISP_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_isp_info_as_json(&dss_info->isp_info[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"ircut\": [\n");
    for (int i = 0; i < DSS_IRCUT_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_dss_hw_ircut_as_json(&dss_info->ircut[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"led\": [\n");
    for (int i = 0; i < DSS_LED_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_dss_hw_led_as_json(&dss_info->led[i]);
        printf("    }");
    }
    printf("\n  ],\n");

    printf("  \"hw_lumi_sensor\": [\n");
    for (int i = 0; i < DSS_MAX_AIN_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("    {\n");
        print_dss_hw_lumen_sensor_as_json(&dss_info->hw_lumi_sensor[i]);
        printf("    }");
    }
    printf("\n  ],\n");
    
    // Print audio_info
    print_ak_qs_audio_info_as_json(&dss_info->audio_info);
    printf(",\n");
    
    // Print npu_info
    printf("  \"npu_info\": [\n");
    for (int i = 0; i < DSS_NPU_MAX_NUM; i++) {
        if (i > 0) printf(",\n");
        print_ak_qs_npu_info_as_json(&dss_info->npu_info[i]);
    }
    printf("\n  ],\n");
    
    // Print little_core_info
    print_ak_little_core_info_as_json(&dss_info->little_core_info);
    printf("\n");
    printf("}\n");
}

// Helper functions for filling structures
int fill_qs_vi_info_from_json(cJSON *json, struct ak_qs_vi_info *vi_info) {
    cJSON *item;
    
    // Process qs_vi_dev_info
    item = cJSON_GetObjectItemCaseSensitive(json, "qs_vi_dev_info");
    if (item) {
        fill_qs_vi_dev_info_from_json(item, &vi_info->qs_vi_dev_info);
    }

    // Process qs_vi_chn_info array
    item = cJSON_GetObjectItemCaseSensitive(json, "qs_vi_chn_info");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_VIDEO_CHN_NUM; i++) {
            cJSON *chn_info_item = cJSON_GetArrayItem(item, i);
            if (chn_info_item) {
                fill_qs_vi_chn_info_from_json(chn_info_item, &vi_info->qs_vi_chn_info[i]);
            }
        }
    }

    return 1;
}

int fill_qs_vi_dev_info_from_json(cJSON *json, struct ak_qs_vi_dev_info *dev_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "dev_id");
    if (item && cJSON_IsNumber(item)) dev_info->dev_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "enable");
    if (item && cJSON_IsNumber(item)) dev_info->enable = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "isp_id");
    if (item && cJSON_IsNumber(item)) dev_info->isp_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "isp_path");
    if (item && cJSON_IsString(item)) {
        strncpy((char*)dev_info->isp_path, item->valuestring, DSS_MAX_PATH_LEN - 1);
        dev_info->isp_path[DSS_MAX_PATH_LEN - 1] = '\0';
    }

    item = cJSON_GetObjectItemCaseSensitive(json, "frame_rate");
    if (item && cJSON_IsNumber(item)) dev_info->frame_rate = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "mirror_en");
    if (item && cJSON_IsNumber(item)) dev_info->mirror_en = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "flip_en");
    if (item && cJSON_IsNumber(item)) dev_info->flip_en = item->valueint;


    item = cJSON_GetObjectItemCaseSensitive(json, "ircut_group_id");
    if (item && cJSON_IsNumber(item)) dev_info->ircut_group_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "led_group_id");
    if (item && cJSON_IsNumber(item)) dev_info->led_group_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "lumen_group_id");
    if (item && cJSON_IsNumber(item)) dev_info->lumen_group_id = item->valueint;

    // Process crop_info
    item = cJSON_GetObjectItemCaseSensitive(json, "crop");
    if (item) {
        fill_crop_info_from_json(item, &dev_info->crop);
    }

    // Process isp_3dnr_info (struct)
    item = cJSON_GetObjectItemCaseSensitive(json, "isp_3dnr_info");
    if (item && cJSON_IsObject(item)) {
        cJSON *total_len_item = cJSON_GetObjectItemCaseSensitive(item, "isp_3dnr_total_len");
        if (total_len_item && cJSON_IsNumber(total_len_item)) {
            dev_info->isp_3dnr_info.isp_3dnr_total_len = total_len_item->valueint;
        }
        cJSON *y_len_item = cJSON_GetObjectItemCaseSensitive(item, "isp_3dnr_y_len");
        if (y_len_item && cJSON_IsNumber(y_len_item)) {
            dev_info->isp_3dnr_info.isp_3dnr_y_len = y_len_item->valueint;
        }
        cJSON *u_len_item = cJSON_GetObjectItemCaseSensitive(item, "isp_3dnr_u_len");
        if (u_len_item && cJSON_IsNumber(u_len_item)) {
            dev_info->isp_3dnr_info.isp_3dnr_u_len = u_len_item->valueint;
        }
        cJSON *v_len_item = cJSON_GetObjectItemCaseSensitive(item, "isp_3dnr_v_len");
        if (v_len_item && cJSON_IsNumber(v_len_item)) {
            dev_info->isp_3dnr_info.isp_3dnr_v_len = v_len_item->valueint;
        }
    }

    // Process fastae_info
    item = cJSON_GetObjectItemCaseSensitive(json, "fastae_info");
    if (item) {
        fill_fastae_info_from_json(item, &dev_info->fastae_info);
    }

    return 1;
}

int fill_qs_vi_chn_info_from_json(cJSON *json, struct ak_qs_vi_chn_info *chn_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "dev_id");
    if (item && cJSON_IsNumber(item)) chn_info->dev_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "chn_id");
    if (item && cJSON_IsNumber(item)) chn_info->chn_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "enable");
    if (item && cJSON_IsNumber(item)) chn_info->enable = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "frame_rate");
    if (item && cJSON_IsNumber(item)) chn_info->frame_rate = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "frame_depth");
    if (item && cJSON_IsNumber(item)) chn_info->frame_depth = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "data_type");
    if (item && cJSON_IsNumber(item)) chn_info->data_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "mode");
    if (item && cJSON_IsNumber(item)) chn_info->mode = item->valueint;

    // Process res (RECTANGLE_S)
    item = cJSON_GetObjectItemCaseSensitive(json, "res");
    if (item && cJSON_IsObject(item)) {
        cJSON *width_item = cJSON_GetObjectItemCaseSensitive(item, "width");
        cJSON *height_item = cJSON_GetObjectItemCaseSensitive(item, "height");
        if (width_item && cJSON_IsNumber(width_item)) chn_info->res.width = width_item->valueint;
        if (height_item && cJSON_IsNumber(height_item)) chn_info->res.height = height_item->valueint;
    }

    // Process crop (crop_info)
    item = cJSON_GetObjectItemCaseSensitive(json, "crop");
    if (item) {
        fill_crop_info_from_json(item, &chn_info->crop);
    }

    // Process max_res (RECTANGLE_S)
    item = cJSON_GetObjectItemCaseSensitive(json, "max_res");
    if (item && cJSON_IsObject(item)) {
        cJSON *width_item = cJSON_GetObjectItemCaseSensitive(item, "width");
        cJSON *height_item = cJSON_GetObjectItemCaseSensitive(item, "height");
        if (width_item && cJSON_IsNumber(width_item)) chn_info->max_res.width = width_item->valueint;
        if (height_item && cJSON_IsNumber(height_item)) chn_info->max_res.height = height_item->valueint;
    }

    // Process stitch_attr
    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_attr");
    if (item) {
        fill_dss_stitch_attr_from_json(item, &chn_info->stitch_attr);
    }

    // Process npu_attr
    item = cJSON_GetObjectItemCaseSensitive(json, "npu_attr");
    if (item) {
        fill_dss_npu_attr_from_json(item, &chn_info->npu_attr);
    }

    return 1;
}

int fill_qs_venc_info_from_json(cJSON *json, struct ak_qs_venc_info *venc_info) {
    cJSON *item;
    
    // Fill all primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "venc_id");
    if (item && cJSON_IsNumber(item)) venc_info->venc_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "width");
    if (item && cJSON_IsNumber(item)) venc_info->width = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "height");
    if (item && cJSON_IsNumber(item)) venc_info->height = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "fps");
    if (item && cJSON_IsNumber(item)) venc_info->fps = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "goplen");
    if (item && cJSON_IsNumber(item)) venc_info->goplen = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "target_kbps");
    if (item && cJSON_IsNumber(item)) venc_info->target_kbps = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "max_kbps");
    if (item && cJSON_IsNumber(item)) venc_info->max_kbps = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "profile");
    if (item && cJSON_IsNumber(item)) venc_info->profile = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "br_mode");
    if (item && cJSON_IsNumber(item)) venc_info->br_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "initqp");
    if (item && cJSON_IsNumber(item)) venc_info->initqp = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "minqp");
    if (item && cJSON_IsNumber(item)) venc_info->minqp = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "maxqp");
    if (item && cJSON_IsNumber(item)) venc_info->maxqp = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "jpeg_qlevel");
    if (item && cJSON_IsNumber(item)) venc_info->jpeg_qlevel = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "chroma_mode");
    if (item && cJSON_IsNumber(item)) venc_info->chroma_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "enc_out_type");
    if (item && cJSON_IsNumber(item)) venc_info->enc_out_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "max_picture_size");
    if (item && cJSON_IsNumber(item)) venc_info->max_picture_size = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "enc_level");
    if (item && cJSON_IsNumber(item)) venc_info->enc_level = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "smart_mode");
    if (item && cJSON_IsNumber(item)) venc_info->smart_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "smart_goplen");
    if (item && cJSON_IsNumber(item)) venc_info->smart_goplen = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "smart_quality");
    if (item && cJSON_IsNumber(item)) venc_info->smart_quality = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "smart_static_value");
    if (item && cJSON_IsNumber(item)) venc_info->smart_static_value = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "gdr_mode");
    if (item && cJSON_IsNumber(item)) venc_info->gdr_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "max_frame_size");
    if (item && cJSON_IsNumber(item)) venc_info->max_frame_size = item->valueint;

    return 1;
}

int fill_qs_video_info_from_json(cJSON *json, struct ak_qs_video_info *video_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "chn_id");
    if (item && cJSON_IsNumber(item)) video_info->chn_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "venc_id");
    if (item && cJSON_IsNumber(item)) video_info->venc_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "enable");
    if (item && cJSON_IsNumber(item)) video_info->enable = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "frame_rate");
    if (item && cJSON_IsNumber(item)) video_info->frame_rate = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "frame_depth");
    if (item && cJSON_IsNumber(item)) video_info->frame_depth = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "venc_encode_mode");
    if (item && cJSON_IsNumber(item)) video_info->venc_encode_mode = item->valueint;

    return 1;
}

int fill_isp_info_from_json(cJSON *json, struct ak_qs_isp_info *isp_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "isp_id");
    if (item && cJSON_IsNumber(item)) isp_info->isp_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "isp_path");
    if (item && cJSON_IsString(item)) {
        strncpy((char*)isp_info->isp_path, item->valuestring, DSS_MAX_PATH_LEN - 1);
        isp_info->isp_path[DSS_MAX_PATH_LEN - 1] = '\0';
    }

    return 1;
}

int fill_dss_hw_ircut_from_json(cJSON *json, struct dss_hw_ircut *ircut) {
    cJSON *item;
    
    // Process ircut_a_gpio
    item = cJSON_GetObjectItemCaseSensitive(json, "ircut_a_gpio");
    if (item) {
        fill_dss_gpio_from_json(item, &ircut->ircut_a_gpio);
    }

    // Process ircut_b_gpio
    item = cJSON_GetObjectItemCaseSensitive(json, "ircut_b_gpio");
    if (item) {
        fill_dss_gpio_from_json(item, &ircut->ircut_b_gpio);
    }

    return 1;
}

int fill_dss_hw_led_from_json(cJSON *json, struct dss_hw_led *led) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "ircut_led_type");
    if (item && cJSON_IsNumber(item)) led->ircut_led_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "whiteled_type");
    if (item && cJSON_IsNumber(item)) led->whiteled_type = item->valueint;

    // Process irled (union)
    item = cJSON_GetObjectItemCaseSensitive(json, "irled_gpio");
    if (item) {
        fill_dss_gpio_from_json(item, &led->irled_gpio);
    } else {
        item = cJSON_GetObjectItemCaseSensitive(json, "irled_pwm");
        if (item) {
            fill_dss_pwm_from_json(item, &led->irled_pwm);
        }
    }

    // Process whiteled (union)
    item = cJSON_GetObjectItemCaseSensitive(json, "whiteled_gpio");
    if (item) {
        fill_dss_gpio_from_json(item, &led->whiteled_gpio);
    } else {
        item = cJSON_GetObjectItemCaseSensitive(json, "whiteled_pwm");
        if (item) {
            fill_dss_pwm_from_json(item, &led->whiteled_pwm);
        }
    }

    return 1;
}

int fill_dss_hw_lumen_sensor_from_json(cJSON *json, struct dss_hw_lumen_sensor *sensor) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "ain_index");
    if (item && cJSON_IsNumber(item)) sensor->ain_index = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "daynight_threshold");
    if (item && cJSON_IsNumber(item)) sensor->daynight_threshold = item->valueint;

    return 1;
}

int fill_crop_info_from_json(cJSON *json, struct qs_crop_info *crop) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "left");
    if (item && cJSON_IsNumber(item)) crop->left = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "top");
    if (item && cJSON_IsNumber(item)) crop->top = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "width");
    if (item && cJSON_IsNumber(item)) crop->width = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "height");
    if (item && cJSON_IsNumber(item)) crop->height = item->valueint;

    return 1;
}

int fill_fastae_info_from_json(cJSON *json, struct fastae_info *fastae) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "bining_mode");
    if (item && cJSON_IsNumber(item)) fastae->bining_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "fastae_mode");
    if (item && cJSON_IsNumber(item)) fastae->fastae_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "ae_stable_range");
    if (item && cJSON_IsNumber(item)) fastae->ae_stable_range = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "max_run_frames");
    if (item && cJSON_IsNumber(item)) fastae->max_run_frames = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "night_mode_light_select");
    if (item && cJSON_IsNumber(item)) fastae->night_mode_light_select = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "daynight_master_id");
    if (item && cJSON_IsNumber(item)) fastae->daynight_master_id = item->valueint;

    // Process daynight_threshold array
    item = cJSON_GetObjectItemCaseSensitive(json, "daynight_threshold");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < 2; i++) {
            cJSON *threshold_item = cJSON_GetArrayItem(item, i);
            if (threshold_item && cJSON_IsNumber(threshold_item)) {
                fastae->daynight_threshold[i] = threshold_item->valueint;
            }
        }
    }

    // Process ae_table_file
    item = cJSON_GetObjectItemCaseSensitive(json, "ae_table_file");
    if (item && cJSON_IsString(item)) {
        strncpy(fastae->ae_table_file, item->valuestring, DSS_MAX_PATH_LEN - 1);
        fastae->ae_table_file[DSS_MAX_PATH_LEN - 1] = '\0';
    }

    return 1;
}

int fill_dss_gpio_from_json(cJSON *json, struct dss_gpio *gpio) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "nb");
    if (item && cJSON_IsNumber(item)) gpio->nb = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "value");
    if (item && cJSON_IsNumber(item)) gpio->value = item->valueint;

    return 1;
}

int fill_dss_pwm_from_json(cJSON *json, struct dss_pwm *pwm) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "pwm_id");
    if (item && cJSON_IsNumber(item)) pwm->pwm_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "duty_ns");
    if (item && cJSON_IsNumber(item)) pwm->duty_ns = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "period_ns");
    if (item && cJSON_IsNumber(item)) pwm->period_ns = item->valueint;

    return 1;
}

int fill_dss_stitch_attr_from_json(cJSON *json, struct dss_stitch_attr *stitch) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_mode");
    if (item && cJSON_IsNumber(item)) stitch->stitch_mode = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_chn_id");
    if (item && cJSON_IsNumber(item)) stitch->stitch_chn_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_num");
    if (item && cJSON_IsNumber(item)) stitch->stitch_num = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_index");
    if (item && cJSON_IsNumber(item)) stitch->stitch_index = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "stitch_global_id");
    if (item && cJSON_IsNumber(item)) stitch->stitch_global_id = item->valueint;

    return 1;
}

int fill_dss_npu_attr_from_json(cJSON *json, struct dss_npu_attr *npu_attr) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "npu_enable");
    if (item && cJSON_IsNumber(item)) npu_attr->npu_enable = item->valueint;

    // Process npu_chn_id array
    item = cJSON_GetObjectItemCaseSensitive(json, "npu_chn_id");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < DSS_NPU_MAX_NUM; i++) {
            cJSON *chn_id_item = cJSON_GetArrayItem(item, i);
            if (chn_id_item && cJSON_IsNumber(chn_id_item)) {
                npu_attr->npu_chn_id[i] = chn_id_item->valueint;
            }
        }
    }

    return 1;
}

int fill_ak_qs_audio_info_from_json(cJSON *json, struct ak_qs_audio_info *audio_info) {
    cJSON *item;
    
    // Process ai_info
    item = cJSON_GetObjectItemCaseSensitive(json, "ai_info");
    if (item) {
        fill_ak_qs_ai_info_from_json(item, &audio_info->ai_info);
    }

    // Process ao_info
    item = cJSON_GetObjectItemCaseSensitive(json, "ao_info");
    if (item) {
        fill_ak_qs_ao_info_from_json(item, &audio_info->ao_info);
    }

    return 1;
}

int fill_ak_qs_ai_info_from_json(cJSON *json, struct ak_qs_ai_info *ai_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "id");
    if (item && cJSON_IsNumber(item)) ai_info->id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "sample_rate");
    if (item && cJSON_IsNumber(item)) ai_info->sample_rate = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "channels");
    if (item && cJSON_IsNumber(item)) ai_info->channels = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "sample_bits");
    if (item && cJSON_IsNumber(item)) ai_info->sample_bits = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "period_bytes");
    if (item && cJSON_IsNumber(item)) ai_info->period_bytes = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "periods");
    if (item && cJSON_IsNumber(item)) ai_info->periods = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "dev_type");
    if (item && cJSON_IsNumber(item)) ai_info->dev_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "gain");
    if (item && cJSON_IsNumber(item)) ai_info->gain = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "data_type");
    if (item && cJSON_IsNumber(item)) ai_info->data_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "reserved");
    if (item && cJSON_IsNumber(item)) ai_info->reserved = item->valueint;

    return 1;
}

int fill_ak_qs_ao_info_from_json(cJSON *json, struct ak_qs_ao_info *ao_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "id");
    if (item && cJSON_IsNumber(item)) ao_info->id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "sample_rate");
    if (item && cJSON_IsNumber(item)) ao_info->sample_rate = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "channels");
    if (item && cJSON_IsNumber(item)) ao_info->channels = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "sample_bits");
    if (item && cJSON_IsNumber(item)) ao_info->sample_bits = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "period_bytes");
    if (item && cJSON_IsNumber(item)) ao_info->period_bytes = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "periods");
    if (item && cJSON_IsNumber(item)) ao_info->periods = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "dev_type");
    if (item && cJSON_IsNumber(item)) ao_info->dev_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "gain");
    if (item && cJSON_IsNumber(item)) ao_info->gain = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "data_type");
    if (item && cJSON_IsNumber(item)) ao_info->data_type = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "data_src");
    if (item && cJSON_IsNumber(item)) ao_info->data_src = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "reserved");
    if (item && cJSON_IsNumber(item)) ao_info->reserved = item->valueint;

    // Process file_path or data_info (union)
    // 根据data_src字段决定使用哪个联合体成员
    if (ao_info->data_src == 0) {  // 文件路径模式
        item = cJSON_GetObjectItemCaseSensitive(json, "file_path");
        if (item && cJSON_IsString(item)) {
            strncpy(ao_info->file_path, item->valuestring, DSS_MAX_PATH_LEN - 1);
            ao_info->file_path[DSS_MAX_PATH_LEN - 1] = '\0';
        }
    } else {  // 数据信息模式
        item = cJSON_GetObjectItemCaseSensitive(json, "data_info");
        if (item && cJSON_IsObject(item)) {
            cJSON *data_addr_item = cJSON_GetObjectItemCaseSensitive(item, "data_addr");
            if (data_addr_item && cJSON_IsNumber(data_addr_item)) {
                ao_info->data_info.data_addr = data_addr_item->valueint;
            }
            cJSON *len_item = cJSON_GetObjectItemCaseSensitive(item, "len");
            if (len_item && cJSON_IsNumber(len_item)) {
                ao_info->data_info.len = len_item->valueint;
            }
        }
    }

    return 1;
}

int fill_ak_qs_npu_info_from_json(cJSON *json, struct ak_qs_npu_info *npu_info) {
    cJSON *item;
    
    // Fill primitive fields
    item = cJSON_GetObjectItemCaseSensitive(json, "chn_id");
    if (item && cJSON_IsNumber(item)) npu_info->chn_id = item->valueint;

    item = cJSON_GetObjectItemCaseSensitive(json, "model_type");
    if (item && cJSON_IsNumber(item)) npu_info->model_type = item->valueint;


    item = cJSON_GetObjectItemCaseSensitive(json, "model_path");
    if (item && cJSON_IsString(item)) {
        strncpy(npu_info->model_path, item->valuestring, DSS_MAX_PATH_LEN - 1);
        npu_info->model_path[DSS_MAX_PATH_LEN - 1] = '\0';
    }

    return 1;
}

int fill_ak_little_core_info_from_json(cJSON *json, struct ak_little_core_info *little_core_info) {
    cJSON *item;
    
    // Fill board_name
    item = cJSON_GetObjectItemCaseSensitive(json, "board_name");
    if (item && cJSON_IsString(item)) {
        strncpy(little_core_info->board_name, item->valuestring, 31);
        little_core_info->board_name[31] = '\0';
    }

    // Process reserved array
    item = cJSON_GetObjectItemCaseSensitive(json, "reserved");
    if (item && cJSON_IsArray(item)) {
        int size = cJSON_GetArraySize(item);
        for (int i = 0; i < size && i < 32; i++) {
            cJSON *reserved_item = cJSON_GetArrayItem(item, i);
            if (reserved_item && cJSON_IsNumber(reserved_item)) {
                little_core_info->reserved[i] = (unsigned char)reserved_item->valueint;
            }
        }
    }

    return 1;
}

// Helper functions for printing structures
void print_qs_vi_info_as_json(struct ak_qs_vi_info *vi_info) {
    printf("      \"qs_vi_dev_info\": {\n");
    print_qs_vi_dev_info_as_json(&vi_info->qs_vi_dev_info);
    printf("      },\n");
    
    printf("      \"qs_vi_chn_info\": [\n");
    for (int i = 0; i < DSS_VIDEO_CHN_NUM; i++) {
        if (i > 0) printf(",\n");
        printf("        {\n");
        print_qs_vi_chn_info_as_json(&vi_info->qs_vi_chn_info[i]);
        printf("        }");
    }
    printf("\n      ]");
}

void print_qs_vi_dev_info_as_json(struct ak_qs_vi_dev_info *dev_info) {
    printf("        \"dev_id\": %d,\n", dev_info->dev_id);
    printf("        \"enable\": %d,\n", dev_info->enable);
    printf("        \"isp_id\": %d,\n", dev_info->isp_id);
    printf("        \"isp_path\": \"%s\",\n", dev_info->isp_path);
    printf("        \"crop\": {\n");
    print_crop_info_as_json(&dev_info->crop);
    printf("        },\n");
    printf("        \"frame_rate\": %d,\n", dev_info->frame_rate);
    printf("        \"mirror_en\": %u,\n", dev_info->mirror_en);
    printf("        \"flip_en\": %u,\n", dev_info->flip_en);
    printf("        \"isp_3dnr_info\": {\n");
    printf("          \"isp_3dnr_total_len\": %d,\n", dev_info->isp_3dnr_info.isp_3dnr_total_len);
    printf("          \"isp_3dnr_y_len\": %d,\n", dev_info->isp_3dnr_info.isp_3dnr_y_len);
    printf("          \"isp_3dnr_u_len\": %d,\n", dev_info->isp_3dnr_info.isp_3dnr_u_len);
    printf("          \"isp_3dnr_v_len\": %d\n", dev_info->isp_3dnr_info.isp_3dnr_v_len);
    printf("        },\n");
    printf("        \"ircut_group_id\": %d,\n", dev_info->ircut_group_id);
    printf("        \"led_group_id\": %d,\n", dev_info->led_group_id);
    printf("        \"lumen_group_id\": %d,\n", dev_info->lumen_group_id);
    printf("        \"fastae_info\": {\n");
    print_fastae_info_as_json(&dev_info->fastae_info);
    printf("        }");
}

void print_qs_vi_chn_info_as_json(struct ak_qs_vi_chn_info *chn_info) {
    printf("          \"dev_id\": %d,\n", chn_info->dev_id);
    printf("          \"chn_id\": %d,\n", chn_info->chn_id);
    printf("          \"enable\": %d,\n", chn_info->enable);
    printf("          \"frame_rate\": %d,\n", chn_info->frame_rate);
    printf("          \"res\": {\n");
    printf("            \"width\": %d,\n", chn_info->res.width);
    printf("            \"height\": %d\n", chn_info->res.height);
    printf("          },\n");
    printf("          \"frame_depth\": %d,\n", chn_info->frame_depth);
    printf("          \"data_type\": %d,\n", chn_info->data_type);
    printf("          \"mode\": %d,\n", chn_info->mode);
    printf("          \"crop\": {\n");
    print_crop_info_as_json(&chn_info->crop);
    printf("          },\n");
    printf("          \"max_res\": {\n");
    printf("            \"width\": %d,\n", chn_info->max_res.width);
    printf("            \"height\": %d\n", chn_info->max_res.height);
    printf("          },\n");
    printf("          \"stitch_attr\": {\n");
    print_dss_stitch_attr_as_json(&chn_info->stitch_attr);
    printf("          },\n");
    printf("          \"npu_attr\": {\n");
    print_dss_npu_attr_as_json(&chn_info->npu_attr);
    printf("          }");
}

void print_qs_venc_info_as_json(struct ak_qs_venc_info *venc_info) {
    printf("        \"venc_id\": %d,\n", venc_info->venc_id);
    printf("        \"width\": %u,\n", venc_info->width);
    printf("        \"height\": %u,\n", venc_info->height);
    printf("        \"fps\": %u,\n", venc_info->fps);
    printf("        \"goplen\": %u,\n", venc_info->goplen);
    printf("        \"target_kbps\": %u,\n", venc_info->target_kbps);
    printf("        \"max_kbps\": %u,\n", venc_info->max_kbps);
    printf("        \"profile\": %u,\n", venc_info->profile);
    printf("        \"br_mode\": %u,\n", venc_info->br_mode);
    printf("        \"initqp\": %u,\n", venc_info->initqp);
    printf("        \"minqp\": %u,\n", venc_info->minqp);
    printf("        \"maxqp\": %u,\n", venc_info->maxqp);
    printf("        \"jpeg_qlevel\": %u,\n", venc_info->jpeg_qlevel);
    printf("        \"chroma_mode\": %u,\n", venc_info->chroma_mode);
    printf("        \"enc_out_type\": %u,\n", venc_info->enc_out_type);
    printf("        \"max_picture_size\": %u,\n", venc_info->max_picture_size);
    printf("        \"enc_level\": %u,\n", venc_info->enc_level);
    printf("        \"smart_mode\": %u,\n", venc_info->smart_mode);
    printf("        \"smart_goplen\": %u,\n", venc_info->smart_goplen);
    printf("        \"smart_quality\": %u,\n", venc_info->smart_quality);
    printf("        \"smart_static_value\": %u,\n", venc_info->smart_static_value);
    printf("        \"gdr_mode\": %u,\n", venc_info->gdr_mode);
    printf("        \"max_frame_size\": %u", venc_info->max_frame_size);
}

void print_qs_video_info_as_json(struct ak_qs_video_info *video_info) {
    printf("        \"chn_id\": %d,\n", video_info->chn_id);
    printf("        \"venc_id\": %d,\n", video_info->venc_id);
    printf("        \"enable\": %d,\n", video_info->enable);
    printf("        \"frame_rate\": %d,\n", video_info->frame_rate);
    printf("        \"frame_depth\": %d,\n", video_info->frame_depth);
    printf("        \"venc_encode_mode\": %d", video_info->venc_encode_mode);
}

void print_isp_info_as_json(struct ak_qs_isp_info *isp_info) {
    printf("        \"isp_id\": %d,\n", isp_info->isp_id);
    printf("        \"isp_path\": \"%s\"", isp_info->isp_path);
}

void print_dss_hw_ircut_as_json(struct dss_hw_ircut *ircut) {
    printf("        \"ircut_a_gpio\": {\n");
    print_dss_gpio_as_json(&ircut->ircut_a_gpio);
    printf("        },\n");
    printf("        \"ircut_b_gpio\": {\n");
    print_dss_gpio_as_json(&ircut->ircut_b_gpio);
    printf("        }");
}

void print_dss_hw_led_as_json(struct dss_hw_led *led) {
    printf("        \"ircut_led_type\": %d,\n", led->ircut_led_type);
    printf("        \"irled_gpio\": {\n");
    print_dss_gpio_as_json(&led->irled_gpio);
    printf("        },\n");
    printf("        \"whiteled_type\": %d,\n", led->whiteled_type);
    printf("        \"whiteled_gpio\": {\n");
    print_dss_gpio_as_json(&led->whiteled_gpio);
    printf("        }");
}

void print_dss_hw_lumen_sensor_as_json(struct dss_hw_lumen_sensor *sensor) {
    printf("        \"ain_index\": %d,\n", sensor->ain_index);
    printf("        \"daynight_threshold\": %d", sensor->daynight_threshold);
}

void print_crop_info_as_json(struct qs_crop_info *crop) {
    printf("          \"left\": %d,\n", crop->left);
    printf("          \"top\": %d,\n", crop->top);
    printf("          \"width\": %d,\n", crop->width);
    printf("          \"height\": %d", crop->height);
}

void print_fastae_info_as_json(struct fastae_info *fastae) {
    printf("          \"bining_mode\": %d,\n", fastae->bining_mode);
    printf("          \"fastae_mode\": %d,\n", fastae->fastae_mode);
    printf("          \"ae_stable_range\": %d,\n", fastae->ae_stable_range);
    printf("          \"max_run_frames\": %d,\n", fastae->max_run_frames);
    printf("          \"night_mode_light_select\": %d,\n", fastae->night_mode_light_select);
    printf("          \"daynight_threshold\": [%d, %d],\n", fastae->daynight_threshold[0], fastae->daynight_threshold[1]);
    printf("          \"daynight_master_id\": %d,\n", fastae->daynight_master_id);
    printf("          \"ae_table_file\": \"%s\"", fastae->ae_table_file);
}

void print_dss_gpio_as_json(struct dss_gpio *gpio) {
    printf("          \"nb\": %d,\n", gpio->nb);
    printf("          \"value\": %d", gpio->value);
}

void print_dss_pwm_as_json(struct dss_pwm *pwm) {
    printf("          \"pwm_id\": %d,\n", pwm->pwm_id);
    printf("          \"duty_ns\": %d,\n", pwm->duty_ns);
    printf("          \"period_ns\": %d", pwm->period_ns);
}

void print_dss_stitch_attr_as_json(struct dss_stitch_attr *stitch) {
    printf("          \"stitch_mode\": %d,\n", stitch->stitch_mode);
    printf("          \"stitch_chn_id\": %d,\n", stitch->stitch_chn_id);
    printf("          \"stitch_num\": %d,\n", stitch->stitch_num);
    printf("          \"stitch_index\": %d,\n", stitch->stitch_index);
    printf("          \"stitch_global_id\": %d", stitch->stitch_global_id);
}

void print_dss_npu_attr_as_json(struct dss_npu_attr *npu_attr) {
    printf("          \"npu_enable\": %d,\n", npu_attr->npu_enable);
    printf("          \"npu_chn_id\": [");
    for (int i = 0; i < DSS_NPU_MAX_NUM; i++) {
        if (i > 0) printf(", ");
        printf("%d", npu_attr->npu_chn_id[i]);
    }
    printf("]");
}

void print_ak_qs_audio_info_as_json(struct ak_qs_audio_info *audio_info) {
    printf("  \"audio_info\": {\n");
    printf("    \"ai_info\": {\n");
    print_ak_qs_ai_info_as_json(&audio_info->ai_info);
    printf("\n    },\n");
    printf("    \"ao_info\": {\n");
    print_ak_qs_ao_info_as_json(&audio_info->ao_info);
    printf("\n    }\n");
    printf("  }");
}

void print_ak_qs_ai_info_as_json(struct ak_qs_ai_info *ai_info) {
    printf("      \"id\": %d,\n", ai_info->id);
    printf("      \"sample_rate\": %u,\n", ai_info->sample_rate);
    printf("      \"channels\": %u,\n", ai_info->channels);
    printf("      \"sample_bits\": %u,\n", ai_info->sample_bits);
    printf("      \"period_bytes\": %u,\n", ai_info->period_bytes);
    printf("      \"periods\": %u,\n", ai_info->periods);
    printf("      \"dev_type\": %d,\n", ai_info->dev_type);
    printf("      \"gain\": %d,\n", ai_info->gain);
    printf("      \"data_type\": %d,\n", ai_info->data_type);
    printf("      \"reserved\": %u", ai_info->reserved);
}

void print_ak_qs_ao_info_as_json(struct ak_qs_ao_info *ao_info) {
    printf("      \"id\": %d,\n", ao_info->id);
    printf("      \"sample_rate\": %u,\n", ao_info->sample_rate);
    printf("      \"channels\": %u,\n", ao_info->channels);
    printf("      \"sample_bits\": %u,\n", ao_info->sample_bits);
    printf("      \"period_bytes\": %u,\n", ao_info->period_bytes);
    printf("      \"periods\": %u,\n", ao_info->periods);
    printf("      \"dev_type\": %d,\n", ao_info->dev_type);
    printf("      \"gain\": %d,\n", ao_info->gain);
    printf("      \"data_type\": %d,\n", ao_info->data_type);
    printf("      \"data_src\": %d,\n", ao_info->data_src);
    
    // 根据data_src字段决定打印哪个联合体成员
    if (ao_info->data_src == 0) {  // 文件路径模式
        printf("      \"file_path\": \"%s\",\n", ao_info->file_path);
    } else {  // 数据信息模式
        printf("      \"data_info\": {\n");
        printf("        \"data_addr\": %u,\n", ao_info->data_info.data_addr);
        printf("        \"len\": %u\n", ao_info->data_info.len);
        printf("      },\n");
    }
    
    printf("      \"reserved\": %u", ao_info->reserved);
}

void print_ak_qs_npu_info_as_json(struct ak_qs_npu_info *npu_info) {
    printf("    {\n");
    printf("      \"chn_id\": %d,\n", npu_info->chn_id);
    printf("      \"model_type\": %d,\n", npu_info->model_type);
    printf("      \"model_path\": \"%s\"\n", npu_info->model_path);
    printf("    }");
}

void print_ak_little_core_info_as_json(struct ak_little_core_info *little_core_info) {
    printf("  \"little_core_info\": {\n");
    printf("    \"board_name\": \"%s\",\n", little_core_info->board_name);
    printf("    \"reserved\": [");
    for (int i = 0; i < 32; i++) {
        if (i > 0) printf(", ");
        printf("%u", little_core_info->reserved[i]);
    }
    printf("]\n");
    printf("  }");
}
