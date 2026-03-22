#ifndef PROXIMA_GEM_H
#define PROXIMA_GEM_H

#include <QString>
#include <QDateTime>
#include <QElapsedTimer>
#include "stdlib/Time.h"
#include "stdlib/Collection.h"
#include "utils/Logger.h"

namespace proxima {

/**
 * @brief Базовый класс GEM (Generic Execution Model)
 * 
 * Оптимизированная реализация:
 * - Минимальное выделение памяти
 * - Inline методы для производительности
 * - get_metrics() - только внутреннее состояние
 * - metrics() - переопределяется пользователем
 * 
 * Согласно language.txt пункт 44
 */
class GEM {
public:
    /**
     * @brief Конструктор
     * @param name Имя объекта (по умолчанию пустое)
     */
    inline GEM(const QString& name = QString()) 
        : objectName_(name)
        , objectType_("gem")
        , initialized_(false)
        , alive_(true)
        , updateCount_(0)
        , execTime_(0.0)
        , minExecTime_(0.0)
        , maxExecTime_(0.0)
        , totalExecTime_(0.0)
        , successCount_(0)
        , failureCount_(0) {
        
        createdAt_ = QDateTime::currentDateTime();
        updatedAt_ = createdAt_;
    }
    
    /**
     * @brief Деструктор
     */
    inline virtual ~GEM() {}
    
    // ========================================================================
    // Основные методы GEM интерфейса (language.txt пункт 44)
    // ========================================================================
    
    /**
     * @brief Инициализация объекта
     * @return true если успешно
     */
    virtual bool init();
    
    /**
     * @brief Сброс объекта
     * @return true если успешно
     */
    virtual bool reset();
    
    /**
     * @brief Обновление состояния
     * @param currentTime Текущее время
     * @return [time_next_call: time, is_alive: bool]
     */
    virtual Collection update(const Time& currentTime);
    
    /**
     * @brief Отображение состояния
     */
    virtual void show();
    
    /**
     * @brief Получение внутренних метрик (базовая реализация)
     * @return Collection с внутренним состоянием
     * 
     * Примечание: Возвращает только внутреннее состояние GEM.
     * Для пользовательских метрик переопределите metrics().
     */
    virtual Collection get_metrics() const;
    
    /**
     * @brief Получение пользовательских метрик (переопределяется)
     * @return Collection с пользовательскими метриками
     * 
     * Примечание: Переопределите этот метод в наследнике для возврата
     * специфичных для объекта метрик.
     */
    virtual Collection metrics() const;
    
    /**
     * @brief Установка параметров
     * @param params Коллекция параметров
     */
    virtual void set_params(const Collection& params);
    
    /**
     * @brief Получение параметров
     * @return Коллекция параметров
     */
    virtual Collection get_params() const;
    
    /**
     * @brief Получение имени
     * @return Имя объекта
     */
    inline QString get_name() const { return objectName_; }
    
    /**
     * @brief Установка имени
     * @param name Имя объекта
     */
    inline void set_name(const QString& name) { objectName_ = name; }
    
    /**
     * @brief Получение типа
     * @return Тип объекта
     */
    inline QString get_type() const { return objectType_; }
    
    /**
     * @brief Установка типа
     * @param type Тип объекта
     */
    inline void set_type(const QString& type) { objectType_ = type; }
    
    /**
     * @brief Проверка инициализации
     * @return true если инициализирован
     */
    inline bool is_initialized() const { return initialized_; }
    
    /**
     * @brief Проверка активности
     * @return true если активен
     */
    inline bool is_alive() const { return alive_; }
    
    /**
     * @brief Установка активности
     * @param alive Статус активности
     */
    inline void set_alive(bool alive) { alive_ = alive; }
    
    /**
     * @brief Получение времени последнего обновления
     * @return Время
     */
    inline Time get_last_update_time() const { return lastUpdateTime_; }
    
    /**
     * @brief Получение количества обновлений
     * @return Количество
     */
    inline int get_update_count() const { return updateCount_; }
    
    /**
     * @brief Получение времени последнего выполнения
     * @return Время в мс
     */
    inline double get_exec_time() const { return execTime_; }
    
    /**
     * @brief Получение среднего времени выполнения
     * @return Время в мс
     */
    inline double get_avg_exec_time() const {
        return (updateCount_ > 0) ? (totalExecTime_ / updateCount_) : 0.0;
    }
    
    /**
     * @brief Получение минимального времени выполнения
     * @return Время в мс
     */
    inline double get_min_exec_time() const { return minExecTime_; }
    
    /**
     * @brief Получение максимального времени выполнения
     * @return Время в мс
     */
    inline double get_max_exec_time() const { return maxExecTime_; }
    
    /**
     * @brief Получение времени создания
     * @return Время
     */
    inline QDateTime get_created_at() const { return createdAt_; }
    
    /**
     * @brief Получение времени последнего обновления
     * @return Время
     */
    inline QDateTime get_updated_at() const { return updatedAt_; }
    
    // ========================================================================
    // Методы публикации и сохранения (опциональные)
    // ========================================================================
    
    /**
     * @brief Публикация отчёта
     * @param doc Документ для публикации
     */
    virtual void publish(const Collection& doc);
    
    /**
     * @brief Сохранение состояния
     * @return Коллекция состояния
     */
    virtual Collection store() const;
    
    /**
     * @brief Восстановление состояния
     * @param state Коллекция состояния
     */
    virtual void restore(const Collection& state);
    
protected:
    // Данные объекта (минимальный набор)
    QString objectName_;        ///< Имя объекта
    QString objectType_;        ///< Тип объекта
    bool initialized_;          ///< Флаг инициализации
    bool alive_;                ///< Флаг активности
    int updateCount_;           ///< Счётчик обновлений
    
    // Временные метки
    Time lastUpdateTime_;       ///< Время последнего обновления
    QDateTime createdAt_;       ///< Время создания
    QDateTime updatedAt_;       ///< Время последнего обновления
    
    // Статистика выполнения (только double, без QMap)
    double execTime_;           ///< Время последнего выполнения (мс)
    double minExecTime_;        ///< Минимальное время (мс)
    double maxExecTime_;        ///< Максимальное время (мс)
    double totalExecTime_;      ///< Общее время (мс)
    int successCount_;          ///< Количество успешных выполнений
    int failureCount_;          ///< Количество неудачных выполнений
    
    // Таймер (один на объект)
    mutable QElapsedTimer timer_;
    
    /**
     * @brief Логирование
     * @param message Сообщение
     * @param level Уровень (0=error, 1=warning, 2=info, 3=debug)
     */
    inline void log(const QString& message, int level = 2) const;
    
    /**
     * @brief Обновление статистики при ошибке
     * @param execTime Время выполнения
     */
    inline void recordFailure(double execTime);
};

} // namespace proxima

#endif // PROXIMA_GEM_H