#include "gem.h"

namespace proxima {

// ============================================================================
// GEM Implementation
// ============================================================================

bool GEM::init() {
    timer_.start();
    
    initialized_ = true;
    alive_ = true;
    lastUpdateTime_ = Time::now();
    updatedAt_ = QDateTime::currentDateTime();
    
    execTime_ = timer_.elapsed() / 1000.0;
    
    log("Initialized", 2);
    
    return true;
}

bool GEM::reset() {
    timer_.start();
    
    // Сброс счётчиков
    updateCount_ = 0;
    execTime_ = 0.0;
    minExecTime_ = 0.0;
    maxExecTime_ = 0.0;
    totalExecTime_ = 0.0;
    successCount_ = 0;
    failureCount_ = 0;
    
    // Сброс состояния
    initialized_ = false;
    alive_ = true;
    updatedAt_ = QDateTime::currentDateTime();
    
    execTime_ = timer_.elapsed() / 1000.0;
    
    log("Reset", 2);
    
    return true;
}

Collection GEM::update(const Time& currentTime) {
    timer_.start();
    
    lastUpdateTime_ = currentTime;
    updateCount_++;
    updatedAt_ = QDateTime::currentDateTime();
    
    execTime_ = timer_.elapsed() / 1000.0;
    totalExecTime_ += execTime_;
    
    if (updateCount_ == 1) {
        minExecTime_ = execTime_;
        maxExecTime_ = execTime_;
    } else {
        if (execTime_ < minExecTime_) minExecTime_ = execTime_;
        if (execTime_ > maxExecTime_) maxExecTime_ = execTime_;
    }
    
    successCount_++;
    
    // Возврат минимальной коллекции
    Collection result;
    result.set("time_next_call", Collection::fromString(currentTime.formatted()));
    result.set("is_alive", Collection::fromBoolean(alive_));
    
    return result;
}

void GEM::show() {
    LOG_INFO("GEM: " + objectName_.toStdString() + 
             " [" + objectType_.toStdString() + "]" +
             " updates=" + std::to_string(updateCount_) +
             " alive=" + (alive_ ? "true" : "false"));
}

Collection GEM::get_metrics() const {
    Collection metrics;
    
    // Базовая информация
    metrics.set("name", Collection::fromString(objectName_));
    metrics.set("type", Collection::fromString(objectType_));
    metrics.set("initialized", Collection::fromBoolean(initialized_));
    metrics.set("alive", Collection::fromBoolean(alive_));
    
    // Счётчики
    metrics.set("update_count", Collection::fromNumber(updateCount_));
    metrics.set("success_count", Collection::fromNumber(successCount_));
    metrics.set("failure_count", Collection::fromNumber(failureCount_));
    
    // Время выполнения
    metrics.set("exec_time", Collection::fromNumber(execTime_));
    metrics.set("min_exec_time", Collection::fromNumber(minExecTime_));
    metrics.set("max_exec_time", Collection::fromNumber(maxExecTime_));
    metrics.set("total_exec_time", Collection::fromNumber(totalExecTime_));
    
    // Временные метки
    metrics.set("created_at", Collection::fromString(createdAt_.toString(Qt::ISODate)));
    metrics.set("updated_at", Collection::fromString(updatedAt_.toString(Qt::ISODate)));
    
    return metrics;
}

Collection GEM::metrics() const {
    // По умолчанию возвращает пустую коллекцию
    // Переопределите в наследнике для возврата пользовательских метрик
    return Collection();
}

void GEM::set_params(const Collection& params) {
    Q_UNUSED(params);
    // Базовая реализация пустая
    // Переопределите в наследнике для обработки параметров
}

Collection GEM::get_params() const {
    // Базовая реализация возвращает пустую коллекцию
    // Переопределите в наследнике для возврата параметров
    return Collection();
}

void GEM::publish(const Collection& doc) {
    Q_UNUSED(doc);
    // Базовая реализация пустая
    // Переопределите в наследнике для публикации
}

Collection GEM::store() const {
    Collection state;
    state.set("name", Collection::fromString(objectName_));
    state.set("type", Collection::fromString(objectType_));
    state.set("initialized", Collection::fromBoolean(initialized_));
    state.set("update_count", Collection::fromNumber(updateCount_));
    return state;
}

void GEM::restore(const Collection& state) {
    objectName_ = state.get("name").toString();
    objectType_ = state.get("type").toString();
    initialized_ = state.get("initialized").toBoolean();
    updateCount_ = state.get("update_count").toNumber();
    updatedAt_ = QDateTime::currentDateTime();
}

// ============================================================================
// Protected Methods
// ============================================================================

void GEM::log(const QString& message, int level) const {
    if (level <= 2) {  // Только error, warning, info
        LOG_INFO("[GEM:" + objectName_.toStdString() + "] " + message.toStdString());
    }
}

void GEM::recordFailure(double execTime) {
    failureCount_++;
    execTime_ = execTime;
    totalExecTime_ += execTime;
    if (updateCount_ == 1) {
        minExecTime_ = execTime;
        maxExecTime_ = execTime;
    } else {
        if (execTime < minExecTime_) minExecTime_ = execTime;
        if (execTime > maxExecTime_) maxExecTime_ = execTime;
    }
}

} // namespace proxima