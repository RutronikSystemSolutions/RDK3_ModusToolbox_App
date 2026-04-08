#ifndef GESTURE_CONTROL_H_
#define GESTURE_CONTROL_H_

#include <stdint.h>

typedef enum
{
    GESTURE_NONE  = 0,
    GESTURE_RIGHT = 1,
    GESTURE_LEFT  = 2,
    GESTURE_UP    = 3,
    GESTURE_DOWN  = 4
} gesture_t;

/* verarbeitet ein neues 3-Kanal-Sample
 * Rückgabe:
 *   GESTURE_NONE  -> kein neues Event
 *   sonst         -> neu erkannte Geste (einmalig)
 */
uint8_t gesture_control_process_sample(uint16_t ps1, uint16_t ps2, uint16_t ps3);

/* liefert die aktuell gehaltene Geste zurück
 * bleibt nach Erkennung 3 Sekunden aktiv
 */
uint8_t gesture_control_get_last_gesture(void);

const char* gesture_control_get_gesture_name(uint8_t gesture);

void gesture_control_reset(void);

#endif /* GESTURE_CONTROL_H_ */