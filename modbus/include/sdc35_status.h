#ifdef SDC35_STATUS_H
#define SDC35_STATUS_H

typedef struct {
    char timestamp[32];
    float pv;
    float sv;
    int config;
    int alarm;
} sdc35status;

#endif