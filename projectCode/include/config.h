// include/config.h
#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

typedef struct {
    // int MIN_TIME;
    float MAX_TIME;
    int MIN_COLUMN;
    int MAX_COLUMN;
    int NUM_GENERATORS;
    int MIN_ROW;
    int MAX_ROW;
    int MIN_VALUE;
    int MAX_VALUE;
    float MISS_PERCENTAGE;
    int NUM_CALCULATORS;
    int NUM_MOVERS;
    int NUM_INSPECTOR1;
    int INSPECTOR1_THRESHOLD;
    int NUM_INSPECTOR2;
    int INSPECTOR2_THRESHOLD;
    int NUM_INSPECTOR3;
    int INSPECTOR3_THRESHOLD;
    int PROCESSED_THRESHOLD;
    int UNPROCESSED_THRESHOLD;
    int BACKUP_THRESHOLD;
    int DELETED_THRESHOLD;
} Config;

int load_config(const char *filename, Config *config);

#endif // CONFIG_H
