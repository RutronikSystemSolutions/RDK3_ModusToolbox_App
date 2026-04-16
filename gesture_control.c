#include "gesture_control.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h> // Wichtig für sqrtf()

/* --- Konfiguration für SCHNELLE Gesten --- */
#define GESTURE_FRAME_SIZE   10    // Feste Blockgröße 
#define GESTURE_FRAME_SKIP   30    // Cooldown-Frames
#define STD_THRESHOLD        35.0f // Schwelle für die Standardabweichung 
#define MIN_ACTIVITY_TH      10.0f // NEU: Minimales Rauschen/Bewegung, um als "aktiv" zu gelten

typedef struct {
    float sensor1[GESTURE_FRAME_SIZE];
    float sensor2[GESTURE_FRAME_SIZE];
    float sensor3[GESTURE_FRAME_SIZE];
    uint8_t buff_pos;      
    uint32_t frames_to_skip;
} gesture_control_state_t;

static gesture_control_state_t g_state;

/* --- Hilfsfunktionen --- */

void gesture_control_reset(void) {
    memset(&g_state, 0, sizeof(g_state));
}

// Berechnet die Standardabweichung eines Arrays
static float calculate_std(const float *data) {
    float mean = 0.0f;
    for (int i = 0; i < GESTURE_FRAME_SIZE; i++) {
        mean += data[i];
    }
    mean /= GESTURE_FRAME_SIZE;
    
    float variance = 0.0f;
    for (int i = 0; i < GESTURE_FRAME_SIZE; i++) {
        variance += (data[i] - mean) * (data[i] - mean);
    }
    return sqrtf(variance / (GESTURE_FRAME_SIZE - 1));
}

// Kreuzkorrelation: Gibt den Lag (Versatz) zurück
static int calculate_lag(const float *a, const float *b) {
    int best_lag = 0;
    float max_corr = -1.0e30f;
    
    for (int lag = -(GESTURE_FRAME_SIZE - 1); lag <= (GESTURE_FRAME_SIZE - 1); lag++) {
        float sum = 0.0f;
        int start_i = (lag < 0) ? -lag : 0;
        int end_i = (lag > 0) ? GESTURE_FRAME_SIZE - lag : GESTURE_FRAME_SIZE;
        
        for (int i = start_i; i < end_i; i++) {
            sum += a[i] * b[i + lag];
        }
        
        if (sum > max_corr) {
            max_corr = sum;
            best_lag = lag;
        }
    }
    return best_lag;
}

/**
 * Hauptverarbeitung: Wird für jedes neue Sensor-Sample aufgerufen.
 */
uint8_t gesture_control_process_sample(uint16_t ps1, uint16_t ps2, uint16_t ps3) {
    
    float raw_1 = (float)ps1;
    float raw_2 = (float)ps2;
    float raw_3 = (float)ps3;

    // 1. Daten in den Block-Puffer schreiben
    if (g_state.buff_pos < GESTURE_FRAME_SIZE) {
        g_state.sensor1[g_state.buff_pos] = raw_1;
        g_state.sensor2[g_state.buff_pos] = raw_2;
        g_state.sensor3[g_state.buff_pos] = raw_3;
        g_state.buff_pos++;
        
        if (g_state.frames_to_skip > 0) {
            g_state.frames_to_skip--;
        }
    }

    // 2. Block ist voll -> Verarbeiten!
    if (g_state.buff_pos == GESTURE_FRAME_SIZE) {
        g_state.buff_pos = 0; 
        uint8_t detected = GESTURE_NONE;

        if (g_state.frames_to_skip == 0) {
            
            // Standardabweichung für alle 3 Sensoren berechnen
            float std_left  = calculate_std(g_state.sensor1);
            float std_right = calculate_std(g_state.sensor2);
            float std_up    = calculate_std(g_state.sensor3);

            // Trigger-Check
            if (std_left >= STD_THRESHOLD || std_right >= STD_THRESHOLD || std_up >= STD_THRESHOLD) {
                
                // --- NEUER CHECK: Ist es NUR PS1? ---
                // Wenn PS2 und PS3 beide keine nennenswerte Bewegung zeigen, war es keine Wischgeste!
                if (std_right < MIN_ACTIVITY_TH && std_up < MIN_ACTIVITY_TH) {
                    return GESTURE_NONE; 
                }
                
                // Priorität 1: Links-Rechts-Auswertung (Sensor 1 vs Sensor 2)
                int left_right_delay = calculate_lag(g_state.sensor1, g_state.sensor2);
                
                if (left_right_delay < 0) {
                    detected = GESTURE_RIGHT;
                } 
                else if (left_right_delay > 0) {
                    detected = GESTURE_LEFT;
                } 
                else {
                    // Priorität 2: Oben-Unten-Auswertung (Sensor 1 vs Sensor 3)
                    int left_up_delay = calculate_lag(g_state.sensor1, g_state.sensor3);
                    
                    if (left_up_delay > 0) {
                        detected = GESTURE_DOWN;
                    } 
                    else if (left_up_delay < 0) {
                        detected = GESTURE_UP;
                    } 
                    else {
                        // Priorität 3: Oben-Unten-Auswertung Backup (Sensor 2 vs Sensor 3)
                        int right_up_delay = calculate_lag(g_state.sensor2, g_state.sensor3);
                        
                        if (right_up_delay > 0) {
                            detected = GESTURE_DOWN;
                        } 
                        else if (right_up_delay < 0) {
                            detected = GESTURE_UP;
                        }
                    }
                }

                // Geste wurde erkannt
                if (detected != GESTURE_NONE) {
                    printf("VCNL4035 (Gesture): G=%u (Std: L=%.1f, R=%.1f, U=%.1f)\r\n", 
                           detected, std_left, std_right, std_up);
                           
                    g_state.frames_to_skip = GESTURE_FRAME_SKIP;
                }
            }
        }
        return detected;
    }

    return GESTURE_NONE;
}