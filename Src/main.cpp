#include "main.h"
#include "stm32g4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include "usb_device.h"
#include "usbd_customhid.h"
#include "nkro.hpp"

extern USBD_HandleTypeDef hUsbDeviceFS;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim6;

uint32_t adc_buffer[1]; // ADC DMA用バッファ
RapidTriggerKeyboard keyboard;

extern "C" void setup()
{
    // キーボード初期化
    keyboard.init();

    // ADCをDMAモードで開始
    // ADCはTIM6のTRGOイベントで自動的に変換を開始し、結果をadc_bufferに転送します
    if (HAL_ADC_Start_DMA(&hadc1, adc_buffer, 1) != HAL_OK) {
        Error_Handler();
    }
    
    // TIM6を開始 (ADCのトリガーソースとして機能)
    if (HAL_TIM_Base_Start(&htim6) != HAL_OK) {
        Error_Handler();
    }
    printf("Setup complete.\n");
}

extern "C" void loop()
{
    // 15キー分スキャン
    for (int i = 0; i < RapidTriggerKeyboard::KEY_COUNT; i++) {
        // 本来はここでマルチプレクサを切り替えて、対応するADC値を読む
        // 今回は「同じADC値」を使うという指示のため、adc_buffer[0]をそのまま使用
        uint32_t val = adc_buffer[0];

        // キー状態の更新
        keyboard.updateKey(i, val);
    }

    // レポートの取得
    KeyboardReport* report = keyboard.getReport();

    // デバッグ出力
    for (int i = 0; i < RapidTriggerKeyboard::KEY_COUNT; i++) {
        if (keyboard.isKeyPressed(i)) {
            printf("%c:ON ", 'a' + i);
        } else {
            printf("%c:OFF ", 'a' + i);
        }
    }
    printf("\n");

    // USBレポートの送信
    USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t*)report, sizeof(KeyboardReport));
    
    // USBのポーリングレートに合わせてウェイトを入れる (例: 1ms)
    HAL_Delay(1);
}