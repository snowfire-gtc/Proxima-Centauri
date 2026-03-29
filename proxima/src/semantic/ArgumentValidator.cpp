#include "ArgumentValidator.h"
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <cstring>

namespace proxima {

// ============================================================================
// Конструктор/Деструктор
// ============================================================================

ArgumentValidator::ArgumentValidator()
    : validationCount(0)
    , successCount(0)
    , failureCount(0) {
    
    registerCustomValidator("isemail", [](const std::any& value) {
        try {
            const std::string& email = std::any_cast<const std::string&>(value);
            size_t atPos = email.find('@');
            size_t dotPos = email.rfind('.');
            return atPos != std::string::npos && dotPos != std::string::npos && 
                   dotPos > atPos && dotPos < email.length() - 1;
        } catch (...) { return false; }
    });
    
    registerCustomValidator("isurl", [](const std::any& value) {
        try {
            const std::string& url = std::any_cast<const std::string&>(value);
            return url.find("http://") == 0 || url.find("https://") == 0;
        } catch (...) { return false; }
    });
    
    registerCustomValidator("isdate", [](const std::any& value) {
        try {
            const std::string& dateStr = std::any_cast<const std::string&>(value);
            return !dateStr.empty();
        } catch (...) { return false; }
    });
    
    registerCustomValidator("isuuid", [](const std::any& value) {
        try {
            const std::string& uuid = std::any_cast<const std::string&>(value);
            if (uuid.length() != 36) return false;
            int hyphens = 0;
            for (char c : uuid) {
                if (c == '-') hyphens++;
                else if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
            }
            return hyphens == 4;
        } catch (...) { return false; }
    });
    
    registerCustomValidator("ishex", [](const std::any& value) {
        try {
            const std::string& hex = std::any_cast<const std::string&>(value);
            if (hex.empty()) return false;
            return hex.find("0x") == 0 || hex.find("#") == 0;
        } catch (...) { return false; }
    });
    
    LOG_INFO("ArgumentValidator initialized");
}

ArgumentValidator::~ArgumentValidator() {
    LOG_DEBUG("ArgumentValidator destroyed");
}


// ============================================================================
// Парсинг конструкции arguments
// ============================================================================

std::vector<ValidationRule> ArgumentValidator::parseArgumentsBlock(
    ArgumentsNodePtr argumentsNode,
    FunctionDeclNodePtr functionNode) {
    
    std::vector<ValidationRule> rules;
    
    if (!argumentsNode) {
        return rules;
    }
    
    for (const auto& directive : argumentsNode->directives) {
        ValidationRule rule;
        rule.argumentName = directive.argumentName;
        rule.line = directive.token.line;
        rule.column = directive.token.column;
        
        for (const auto& validator : directive.validators) {
            ValidationRule validatorRule = rule;
            validatorRule.validator = stringToValidatorType(validator.name);
            validatorRule.validatorName = validator.name;
            validatorRule.parameters = validator.parameters;
            
            if (!validator.errorMessage.empty()) {
                validatorRule.errorMessage = validator.errorMessage;
            }
            
            rules.push_back(validatorRule);
        }
        
        rule.isOptional = directive.isOptional;
        
        if (directive.hasDefaultValue) {
            rule.defaultValue = directive.defaultValue;
        }
        
        rules.push_back(rule);
    }
    
    LOG_DEBUG("Parsed " + std::to_string(rules.size()) + " validation rules");
    
    return rules;
}

std::vector<ValidationRule> ArgumentValidator::parseValidationAnnotations(
    const std::string& commentText,
    const std::string& argName) {
    
    std::vector<ValidationRule> rules;
    
    size_t pos = 0;
    while ((pos = commentText.find("@validate", pos)) != std::string::npos) {
        pos += 9;
        while (pos < commentText.length() && std::isspace(commentText[pos])) pos++;
        
        size_t colonPos = commentText.find(':', pos);
        if (colonPos == std::string::npos) continue;
        
        std::string foundArgName = commentText.substr(pos, colonPos - pos);
        size_t start = foundArgName.find_first_not_of(" \t");
        size_t end = foundArgName.find_last_not_of(" \t");
        if (start != std::string::npos) {
            foundArgName = foundArgName.substr(start, end - start + 1);
        }
        
        if (foundArgName == argName) {
            size_t validatorsStart = colonPos + 1;
            size_t validatorsEnd = commentText.find('\n', validatorsStart);
            if (validatorsEnd == std::string::npos) {
                validatorsEnd = commentText.length();
            }
            
            std::string validators = commentText.substr(validatorsStart, validatorsEnd - validatorsStart);
            
            size_t vPos = 0;
            while (vPos < validators.length()) {
                size_t commaPos = validators.find(',', vPos);
                std::string validator;
                if (commaPos == std::string::npos) {
                    validator = validators.substr(vPos);
                    vPos = validators.length();
                } else {
                    validator = validators.substr(vPos, commaPos - vPos);
                    vPos = commaPos + 1;
                }
                
                start = validator.find_first_not_of(" \t");
                end = validator.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    validator = validator.substr(start, end - start + 1);
                }
                
                if (!validator.empty()) {
                    ValidationRule parsedRule = parseValidationAnnotation(validator);
                    if (parsedRule.validator != ValidatorType::Custom) {
                        parsedRule.argumentName = argName;
                        rules.push_back(parsedRule);
                    }
                }
            }
        }
    }
    
    return rules;
}

ArgumentValidator::ValidationRule ArgumentValidator::parseValidationAnnotation(
    const std::string& annotationText) {
    
    ValidationRule rule;
    
    size_t parenPos = annotationText.find('(');
    if (parenPos != std::string::npos) {
        rule.validatorName = annotationText.substr(0, parenPos);
        size_t closeParenPos = annotationText.rfind(')');
        if (closeParenPos != std::string::npos && closeParenPos > parenPos) {
            std::string paramsStr = annotationText.substr(parenPos + 1, closeParenPos - parenPos - 1);
            rule.parameters = extractAnnotationParameters(paramsStr);
        }
    } else {
        rule.validatorName = annotationText;
    }
    
    rule.validator = stringToValidatorType(rule.validatorName);
    
    return rule;
}

std::any ArgumentValidator::extractAnnotationParameters(const std::string& annotationText) {
    std::vector<std::any> params;
    std::stringstream ss(annotationText);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        size_t start = token.find_first_not_of(" \t");
        size_t end = token.find_last_not_of(" \t");
        if (start != std::string::npos) {
            token = token.substr(start, end - start + 1);
        } else {
            continue;
        }
        
        if (token == "true") {
            params.push_back(true);
        } else if (token == "false") {
            params.push_back(false);
        } else {
            try {
                size_t pos;
                double d = std::stod(token, &pos);
                if (pos == token.length()) {
                    if (d == static_cast<int>(d)) {
                        params.push_back(static_cast<int>(d));
                    } else {
                        params.push_back(d);
                    }
                    continue;
                }
            } catch (...) {}
            
            if (token.front() == '"' && token.back() == '"') {
                params.push_back(token.substr(1, token.length() - 2));
            } else {
                params.push_back(token);
            }
        }
    }
    
    if (params.size() == 1) {
        return params[0];
    } else if (params.size() > 1) {
        return params;
    }
    
    return std::any();
}

