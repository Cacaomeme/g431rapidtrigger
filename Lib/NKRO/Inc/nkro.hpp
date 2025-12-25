#ifndef NKRO_HPP
#define NKRO_HPP

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// NKRO対応のレポート構造体 (17バイト)
typedef struct {
    uint8_t MODIFIER;
    uint8_t RESERVED;
    uint8_t KEYS[15]; // 120 keys / 8 = 15 bytes
} KeyboardReport;

// Rapid Trigger State for a single key
struct RapidTriggerState {
    uint32_t high_peak;      // 押し込みの最深点
    uint32_t low_peak;       // 戻りの最浅点
    bool is_active;          // 現在のキー状態
    
    // 設定パラメータ (ADC値 0-4095)
    uint32_t min_threshold;  // これ以下の値ではONにならない（デッドゾーン）
    uint32_t sensitivity;    // ON/OFFに必要な変化量 (Rapid Trigger感度)
};

class RapidTriggerKeyboard {
public:
    static const int KEY_COUNT = 15;

    RapidTriggerKeyboard();
    void init();
    
    // ADC値を更新し、内部状態を更新する
    void updateKey(int keyIndex, uint32_t adcValue);
    
    // 現在の状態からレポートを生成して返す
    KeyboardReport* getReport();

    // デバッグ用：キーの状態を取得
    bool isKeyPressed(int keyIndex) const;

private:
    RapidTriggerState keyStates[KEY_COUNT];
    KeyboardReport report;
    
    // キーコードマップ ('a' -> 'o')
    const uint8_t keyMap[KEY_COUNT] = {
        0x04, 0x05, 0x06, 0x07, 0x08, // a, b, c, d, e
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, // f, g, h, i, j
        0x0E, 0x0F, 0x10, 0x11, 0x12  // k, l, m, n, o
    };

    void updateRapidTriggerState(RapidTriggerState& state, uint32_t currentVal);
};

#endif // NKRO_HPP
