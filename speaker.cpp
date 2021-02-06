#include "speaker.h"

#include <Arduino.h>
#include <melody_factory.h>
#include <melody_player.h>

#define SPEAKER_PIN 15

// Note: the MelodyPlayer library use the channel 0 (for PWM) by default
MelodyPlayer player(SPEAKER_PIN);
String notes[] = {
    "AS4,1", "AS4,1", "AS4,1",
    "F5,4", "C6,4",
    "AS5,1", "A5,1", "G5,1", "F6,4", "C6,2",
    "AS5,1", "A5,1", "G5,1", "F6,4", "C6,2",
    "AS5,1", "A5,1", "AS5,1", "G5,4", "C5,1", "C5,1", "C5,1",
    "F5,4", "C6,4",
    "AS5,1", "A5,1", "G5,1", "F6,4", "C6,2",

    "AS5,1", "A5,1", "G5,1", "F6,4", "C6,2",
    "AS5,1", "A5,1", "AS5,1", "G5,4", "C5,1.", "C5,0.5",
    "D5,2.", "D5,1", "AS5,1", "A5,1", "G5,1", "F5,1",
    "F5,1", "G5,1", "A5,1", "G5,2", "D5,1", "E5,2", "C5,1.", "C5,0.5",
    "D5,2.", "D5,1", "AS5,1", "A5,1", "G5,1", "F5,1",

    "C6,1.", "G5,0.5", "G5,4", "SILENCE,1", "C5,1",
    "D5,2.", "D5,1", "AS5,1", "A5,1", "G5,1", "F5,1",
    "F5,1", "G5,1", "A5,1", "G5,2", "D5,1", "E5,2", "C6,1.", "C6,0.5",
    "F6,2", "DS6,1", "CS6,2", "C6,1", "AS5,2", "GS5,1", "G5,2", "F5,1",
    "C6,8"};
Melody melody = MelodyFactory.load("Star Wars Theme", 225, notes, sizeof(notes) / sizeof(notes[0]));
static bool alarmStoppedByUser = false;

void speaker_play(bool alarm) {
    // if user stopped the alarm, don't play another alarm
    if (alarmStoppedByUser) {
        return;
    }

    // if playing a new alarm, need to reset the variable
    if (alarm) {
        alarmStoppedByUser = false;
    }
    player.stop();
    player.playAsync(melody);
}

void speaker_stop() {
    alarmStoppedByUser = true;
    player.stop();
}

void speaker_resetalarm() {
    alarmStoppedByUser = false;
}

bool speaker_isPlaying() {
    return player.isPlaying();
}