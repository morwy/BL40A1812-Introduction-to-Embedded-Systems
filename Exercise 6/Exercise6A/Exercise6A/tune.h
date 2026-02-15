#ifndef TUNE_H_
#define TUNE_H_

#define BPM (90)
#define FULL (60000/BPM)
#define HALF (FULL/2)
#define QUARTER (FULL/4)

#define C3 262
#define D3 293
#define E3 330
#define F3 349
#define G3 392
#define A3 440
#define B3 494
#define C4 523

typedef struct note {
    uint16_t frequency_hz;
    uint16_t duration_ms;
} note_t;

#endif /* TUNE_H_ */