#include "css_parser.hpp"
#include <cctype>

namespace dong::dom {

std::vector<LayerRule> CSSParser::parseLayerRules(const std::string& css) {
    std::vector<LayerRule> result;
    std::string css_clean = removeComments(css);

    size_t pos = 0;
    int layer_order_counter = 0;

    while (pos < css_clean.length()) {
        // 查找@layer声明
        size_t layer_start = css_clean.find("@layer", pos);
        if (layer_start == std::string::npos) break;

        size_t name_start = layer_start + 6; // "@layer"的长�?

        // 跳过空白字符
        while (name_start < css_clean.length() &&
               std::isspace(static_cast<unsigned char>(css_clean[name_start]))) {
            ++name_start;
        }

        if (name_start >= css_clean.length()) break;

        // 检查是否有开括号（表示带规则的层�?
        size_t brace_open = css_clean.find('{', name_start);
        size_t semicolon = css_clean.find(';', name_start);

        // 处理预声明层（@layer name; �?@layer name1, name2;�?
        if (semicolon != std::string::npos && (brace_open == std::string::npos || semicolon < brace_open)) {
            std::string layer_names = css_clean.substr(name_start, semicolon - name_start);
            layer_names = trimWhitespace(layer_names);

            // 解析多个层名（逗号分隔�?
            size_t comma_pos = 0;
            size_t comma_next = layer_names.find(',', comma_pos);

            while (true) {
                std::string layer_name;
                if (comma_next == std::string::npos) {
                    layer_name = trimWhitespace(layer_names.substr(comma_pos));
                } else {
                    layer_name = trimWhitespace(layer_names.substr(comma_pos, comma_next - comma_pos));
                }

                if (!layer_name.empty()) {
                    LayerRule layer(layer_name, layer_order_counter++, true);
                    result.push_back(layer);
                }

                if (comma_next == std::string::npos) break;
                comma_pos = comma_next + 1;
                comma_next = layer_names.find(',', comma_pos);
            }

            pos = semicolon + 1;
            continue;
        }

        // 处理带规则的层（@layer name { ... } �?@layer { ... }�?
        if (brace_open != std::string::npos) {
            // 提取层名（匿名层为空字符串）
            std::string layer_name;
            if (name_start < brace_open) {
                layer_name = trimWhitespace(css_clean.substr(name_start, brace_open - name_start));
            }

            // 查找匹配的闭括号
            int depth = 1;
            size_t end = brace_open + 1;
            while (end < css_clean.length() && depth > 0) {
                if (css_clean[end] == '{') ++depth;
                else if (css_clean[end] == '}') --depth;
                ++end;
            }

            if (depth > 0) break; // 括号不匹�?

            std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);

            // 解析层内的CSS规则
            LayerRule layer(layer_name, layer_order_counter++, false);
            CSSParser inner_parser;
            layer.rules = inner_parser.parse(content);

            result.push_back(layer);
            pos = end;
        } else {
            // 无效语法，跳�?
            pos = name_start + 1;
        }
    }

    return result;
}


std::vector<KeyframesRule> CSSParser::parseKeyframes(const std::string& css) {
    std::vector<KeyframesRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@keyframes", pos)) != std::string::npos) {
        size_t name_start = pos + 10;
        while (name_start < css_clean.length() && 
               std::isspace(static_cast<unsigned char>(css_clean[name_start]))) {
            ++name_start;
        }
        
        size_t brace_open = css_clean.find('{', name_start);
        if (brace_open == std::string::npos) break;
        
        std::string name = trimWhitespace(css_clean.substr(name_start, brace_open - name_start));
        
        // Find matching closing brace
        int depth = 1;
        size_t end = brace_open + 1;
        while (end < css_clean.length() && depth > 0) {
            if (css_clean[end] == '{') ++depth;
            else if (css_clean[end] == '}') --depth;
            ++end;
        }
        
        std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);
        
        KeyframesRule keyframes;
        keyframes.name = name;
        
        // Parse keyframe blocks
        size_t kf_pos = 0;
        while (kf_pos < content.length()) {
            size_t kf_brace = content.find('{', kf_pos);
            if (kf_brace == std::string::npos) break;
            
            size_t kf_close = content.find('}', kf_brace);
            if (kf_close == std::string::npos) break;
            
            std::string offset_str = trimWhitespace(content.substr(kf_pos, kf_brace - kf_pos));
            std::string props_str = content.substr(kf_brace + 1, kf_close - kf_brace - 1);
            
            CSSKeyframe keyframe;
            if (offset_str == "from") {
                keyframe.offset = 0.0f;
            } else if (offset_str == "to") {
                keyframe.offset = 1.0f;
            } else {
                keyframe.offset = parseFloat(offset_str) / 100.0f;
            }
            
            auto decls = splitDeclarations(props_str);
            for (const auto& decl : decls) {
                size_t colon = decl.find(':');
                if (colon != std::string::npos) {
                    std::string prop = trimWhitespace(decl.substr(0, colon));
                    std::string value = trimWhitespace(decl.substr(colon + 1));
                    keyframe.properties[prop] = value;
                }
            }
            
            keyframes.keyframes.push_back(keyframe);
            kf_pos = kf_close + 1;
        }
        
        result.push_back(keyframes);
        keyframes_map_[name] = keyframes;
        pos = end;
    }
    
    return result;
}


std::vector<MediaRule> CSSParser::parseMediaRules(const std::string& css) {
    std::vector<MediaRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@media", pos)) != std::string::npos) {
        size_t brace_open = css_clean.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        std::string query = trimWhitespace(css_clean.substr(pos + 6, brace_open - pos - 6));
        
        // Find matching closing brace
        int depth = 1;
        size_t end = brace_open + 1;
        while (end < css_clean.length() && depth > 0) {
            if (css_clean[end] == '{') ++depth;
            else if (css_clean[end] == '}') --depth;
            ++end;
        }
        
        std::string content = css_clean.substr(brace_open + 1, end - brace_open - 2);
        
        MediaRule media;
        media.query = query;
        
        // Parse rules inside media block
        CSSParser inner_parser;
        media.rules = inner_parser.parse(content);
        
        result.push_back(media);
        pos = end;
    }
    
    return result;
}


std::vector<FontFaceRule> CSSParser::parseFontFaceRules(const std::string& css) {
    std::vector<FontFaceRule> result;
    std::string css_clean = removeComments(css);
    
    size_t pos = 0;
    while ((pos = css_clean.find("@font-face", pos)) != std::string::npos) {
        size_t brace_open = css_clean.find('{', pos);
        if (brace_open == std::string::npos) break;
        
        size_t brace_close = css_clean.find('}', brace_open);
        if (brace_close == std::string::npos) break;
        
        std::string content = css_clean.substr(brace_open + 1, brace_close - brace_open - 1);
        
        FontFaceRule font_face;
        
        auto decls = splitDeclarations(content);
        for (const auto& decl : decls) {
            size_t colon = decl.find(':');
            if (colon != std::string::npos) {
                std::string prop = trimWhitespace(decl.substr(0, colon));
                std::string value = trimWhitespace(decl.substr(colon + 1));
                
                if (prop == "font-family") {
                    // Remove quotes
                    if ((value.front() == '"' && value.back() == '"') ||
                        (value.front() == '\'' && value.back() == '\'')) {
                        value = value.substr(1, value.length() - 2);
                    }
                    font_face.family = value;
                } else if (prop == "src") {
                    font_face.src = value;
                } else if (prop == "font-style") {
                    font_face.style = value;
                } else if (prop == "font-weight") {
                    font_face.weight = value;
                }
            }
        }
        
        result.push_back(font_face);
        pos = brace_close + 1;
    }
    
    return result;
}

} // namespace dong::dom
