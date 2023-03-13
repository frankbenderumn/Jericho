#ifndef JERICHO_IRIS_FORM_H_
#define JERICHO_IRIS_FORM_H_

#include <string>

#include "iris/defs.h"
#include "picojson.h"

enum FormParam {
    FORM_TEXTFIELD,
    FORM_PASSWORD,
    FORM_SUBMIT,
    FORM_HIDDEN,
    FORM_SELECT,
    FORM_OPTION,
    FORM_CHECKBOX,
    FORM_RADIO,
    FORM_TEXTAREA
};

namespace iris {

    std::string parseFormParams(FormParam param, picojson::value val) {
        std::string input;
        picojson::object o;
        if (!val.is<picojson::object>()) {
            BRED("IrisForm::parseParams: Param not a json object\n");
        }
        o = val.get<picojson::object>();
        switch (param) {
            case FORM_TEXTFIELD:
                input += "\t<input type='textfield'";
                break;
            case FORM_PASSWORD:
                input += "\t<input type='password'";
                break;
            case FORM_SUBMIT:
                input += "\t<input type='submit'";
                break;
        }
        for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
            input += " " + it->first + "='";
            if (it->second.is<std::string>()) {
                input += it->second.get<std::string>();
            } else {
                BRED("IrisForm::parseParams: std::string only supported for now!\n");
            }
            input += "'";
        }
        input += " />\n";
        return input;
    }

    std::string parseForm(LEX lex) {
        BBLU("IRIS FORM DEBUG\n========================================\n");
        printf("%s\n", lex.first.c_str());
        std::string bytes = lex.first;
        bool escaped = false;
        bool endBlock = true;
        int brackets = 0;
        int bracketPtr = 0;
        bool key = false;
        bool attr = false;
        std::string keyStr;
        std::string attrStr;
        std::string json;
        bool initBracket = true;
        bool sqbracket = false;
        bool action = false;
        std::string actionStr;
        std::vector<std::pair<std::string, std::string>> inputs;
        for (auto c : bytes) {         
            if (c == '[') { 
                if (initBracket) {
                    sqbracket = true; 
                } else {
                    json += c;
                }
            }   
            if (c == ']') {
                if (initBracket) {
                    sqbracket = false;
                    initBracket = false;
                    action = false;
                } else {
                    json += c;
                }
            }
            if (c == '=') {
                if (sqbracket) {
                    action = true;
                    continue;
                } else {
                    json += c;
                }
            }
            if (c == '"') { json += c; escaped = !escaped; continue; }
            if (c == '{') { 
                if (!escaped) {
                    brackets++;
                    json += c;
                    continue;
                }
            }
            if (c == '}') { 
                if (!escaped) {
                    brackets--;
                    json += c;
                    if (brackets == 0) { break; }
                    continue;
                }
            }

            if (action) {
                actionStr += c;
            }

            if (brackets != 0) {
                json += c;
            }
        }

        BLU("Json String:\n%s\n", json.c_str());

        picojson::value data;
        if (JFS::parseJson(data, json.c_str()) < 0) {
            BRED("IrisForm: Invalid Json detected\n");
            return "undefined";
        }

        picojson::object o;
        if (data.is<picojson::object>()) {
            o = data.get<picojson::object>();
        }

        BBLU("ActionStr: %s\n", actionStr.c_str());

        std::string csrf_token = "dfsdkfhj4jh3489sdyhksdf";

        std::string form = "<form action='" + actionStr + "' method='POST'>\n";

        for (picojson::object::const_iterator it = o.begin(); it != o.end(); it++) {
            if (it->first == "textfield") {
                BBLU("IrisForm: textfield detected\n");
                std::string result = parseFormParams(FORM_TEXTFIELD, it->second);
                form += result;
            } else if (it->first == "password") {
                BBLU("IrisForm: password detected\n");
                std::string result = parseFormParams(FORM_PASSWORD, it->second);
                form += result;
            } else if (it->first == "submit") {
                BBLU("IrisForm: submit detected\n");
                std::string result = parseFormParams(FORM_SUBMIT, it->second);
                form += result;
            } else {
                BRED("IrisForm: Undetected form attribute\n");
            }
        }

        form += "\t<input type='hidden' csrf-token='" + csrf_token + "' />\n";

        form += "</form>\n";

        BBLU("========================================\n");
        return form;
    }
}


#endif