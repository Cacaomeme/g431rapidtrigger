#include "nkro.hpp"

RapidTriggerKeyboard::RapidTriggerKeyboard() {
    memset(&report, 0, sizeof(report));
}

void RapidTriggerKeyboard::init() {
    for(int i=0; i<KEY_COUNT; i++) {
        keyStates[i].high_peak = 0;
        keyStates[i].low_peak = 4095;
        keyStates[i].is_active = false;
        keyStates[i].min_threshold = 500;
        keyStates[i].sensitivity = 100;
    }
}

void RapidTriggerKeyboard::updateKey(int keyIndex, uint32_t adcValue) {
    if (keyIndex < 0 || keyIndex >= KEY_COUNT) return;

    updateRapidTriggerState(keyStates[keyIndex], adcValue);
}

KeyboardReport* RapidTriggerKeyboard::getReport() {
    // レポートをクリア
    memset(&report, 0, sizeof(report));

    for (int i = 0; i < KEY_COUNT; i++) {
        if (keyStates[i].is_active) {
            uint8_t keycode = keyMap[i];
            // keycodeに対応するビットを立てる
            int byte_index = keycode / 8;
            int bit_index = keycode % 8;
            if (byte_index < 15) {
                report.KEYS[byte_index] |= (1 << bit_index);
            }
        }
    }
    return &report;
}

bool RapidTriggerKeyboard::isKeyPressed(int keyIndex) const {
    if (keyIndex < 0 || keyIndex >= KEY_COUNT) return false;
    return keyStates[keyIndex].is_active;
}

void RapidTriggerKeyboard::updateRapidTriggerState(RapidTriggerState& state, uint32_t currentVal) {
    // 1. ピークの更新
    if (currentVal > state.high_peak) state.high_peak = currentVal;
    if (currentVal < state.low_peak) state.low_peak = currentVal;

    if (state.is_active) {
        // 現在ONの場合 -> OFF判定 (Reset)
        // 「最深点」から「感度」分だけ戻ったらOFF
        if (currentVal < state.high_peak - state.sensitivity) {
            state.is_active = false;
            state.low_peak = currentVal; // 現在位置を新しい「最浅点」の基準にする
        }
    } else {
        // 現在OFFの場合 -> ON判定 (Actuation)
        // 1. 絶対値が最低閾値を超えている
        // 2. 「最浅点」から「感度」分だけ押し込まれた
        if (currentVal > state.min_threshold && 
            currentVal > state.low_peak + state.sensitivity) {
            state.is_active = true;
            state.high_peak = currentVal; // 現在位置を新しい「最深点」の基準にする
        }
        
        // (安全策) 絶対閾値を下回ったら強制的にリセット
        if (currentVal < state.min_threshold) {
             state.is_active = false;
             state.high_peak = state.min_threshold;
             state.low_peak = currentVal;
        }
    }
}
