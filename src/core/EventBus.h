// ============================================================================
//  EventBus.h — Шина событий VARSYS (publish/subscribe)
//
//  Развязывает модули: ядро, UI и будущие подсистемы (радио, ИК, NFC)
//  обмениваются сообщениями, не зная друг о друге напрямую.
// ============================================================================
#pragma once
#include <Arduino.h>
#include <functional>
#include <vector>

// Идентификаторы событий системы. Расширяется по мере роста прошивки.
enum class EventType : uint16_t {
    NONE = 0,

    // --- Системные ---
    SYS_BOOT_DONE,          // Ядро инициализировано
    SYS_LOW_BATTERY,

    // --- Ввод (энкодер) ---
    INPUT_ENCODER_CW,       // Поворот по часовой
    INPUT_ENCODER_CCW,      // Поворот против часовой
    INPUT_BTN_CLICK,        // Короткое нажатие энкодера
    INPUT_BTN_LONG,         // Долгое нажатие энкодера
    INPUT_BACK,             // Кнопка «назад» (боковая)

    // --- Навигация UI ---
    UI_SCREEN_CHANGED,

    // --- Питание ---
    POWER_CHANGED,          // Обновились данные батареи

    // --- Настройки ---
    SETTINGS_CHANGED,       // Любая настройка изменена (i32 = ключ, опц.)
    LANG_CHANGED,           // Сменился язык интерфейса
    UI_REBUILD,             // Запрос пересборки экранов (напр., режим эксперта)

    EVENT_TYPE_COUNT
};

// Полезная нагрузка события (минимальная, универсальная).
struct Event {
    EventType type = EventType::NONE;
    int32_t   i32  = 0;     // числовой параметр (например, дельта энкодера)
    void*     ptr  = nullptr;
};

using EventHandler = std::function<void(const Event&)>;

class EventBus {
public:
    // Подписка на конкретный тип события.
    static void subscribe(EventType type, EventHandler handler);

    // Синхронная публикация (вызывает подписчиков немедленно).
    static void publish(const Event& event);
    static void publish(EventType type, int32_t i32 = 0, void* ptr = nullptr);

    // Отложенная публикация: событие ставится в очередь и доставляется в
    // следующем проходе Core::loop() через dispatchDeferred(). Безопасно для
    // случаев, когда обработчик меняет UI/подписки (без реентерабельности).
    static void publishDeferred(EventType type, int32_t i32 = 0, void* ptr = nullptr);

    // Доставить накопленные отложенные события. Вызывается из Core::loop().
    static void dispatchDeferred();

private:
    struct Subscription {
        EventType    type;
        EventHandler handler;
    };
    static std::vector<Subscription> _subs;
    static std::vector<Event>        _deferred;
};
