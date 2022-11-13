#include <map>
#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <iostream>
#include <Eigen/Dense>

// gloal help functions
#define lupu_assert(Expr, Msg) \
    lupu__M_Assert(#Expr, Expr, __FILE__, __LINE__, Msg)

#define lupu_panic(Msg) \
    lupu__M_Panic(__FILE__, __LINE__, Msg)

inline void lupu__M_Assert(const char* expr_str, bool expr, const char* file, int line, const char* msg) {
    if (!expr) {
        std::cerr << "Assert failed:\t" << msg << "\n"
            << "Expected:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n";
        abort();
    }
}

inline void lupu__M_Panic(const char* file, int line, const char* msg) {
    std::cerr << "Assert failed:\t" << msg << "\n"
        << "Source:\t\t" << file << ", line " << line << "\n";
    abort();
}


using Eigen::MatrixXf;

namespace lupu {
// target number type
using Vec = MatrixXf;
using TNT = float;

struct Cell {
    enum CellType {
        T_Number,
        T_String,
        T_Vector,
    };
    const CellType type_;

    union {
        TNT _num;
        const char* _str;
        const Vec* _vec;
    } v;

    // constructors
    Cell() : type_(T_Number){
        v._num = 0.0;
    }
    Cell(TNT value): type_(T_Number) {
        v._num = value;
    }
    Cell(const char* str): type_(T_String) {
        v._str = str;
    }
    Cell(Vec* vec): type_(T_Vector) {
        v._vec = vec;
    }

    // fast access
    const char* as_string() {
        lupu_assert(type_ == T_String, "Cell type can't convert to string!");
        return v._str;
    }
    bool as_boolean() {
        lupu_assert(type_ == T_Number, "Cell type can't convert to boolean!");
        if ( v._num == 0.0) {
            return false;
        }
        return true;
    }
    TNT as_number() {
        lupu_assert(type_ == T_Number, "Cell type can't convert to number!");
        return v._num;
    }
    const Vec* as_vector() {
        lupu_assert(type_ == T_Vector, "Cell type can't convert to vector!");
        return v._vec;
    }
    bool is_null() {
        if ( type_ == T_String ) {
            if (v._str == NULL) {
                return true;
            }
        }
        return false;
    }
};


// Stack & Hash
struct Stack {
    Stack() {}
    ~Stack() {}

    size_t size() {
        return data_.size();
    }
    void clear() {
        data_.clear();
    }

    Cell top() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        return data_.back();
    }
    Cell pop() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        auto ret =  data_.back();
        data_.pop_back();
        return ret;
    }
    void drop() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        data_.pop_back();
    }
    void dup() {
        data_.push_back( top() );
    }
    void dup2() {
        auto a = pop();
        auto b = pop();
        for(int i = 0; i < 3; i++) {
            data_.push_back(b);
            data_.push_back(a);
        }
    }
    void swap() {
        auto a = pop();
        auto b = pop();
        data_.push_back(a);
        data_.push_back(b);
    }
    void rot() {
        auto a = pop();
        auto b = pop();
        auto c = pop();
        data_.push_back(b);
        data_.push_back(a);
        data_.push_back(c);
    }
    TNT pop_number() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        auto ret =  data_.back();
        data_.pop_back();
        return ret.as_number();
    }
    std::vector<TNT> pop_number_list() {
        size_t s = (size_t) pop().as_number();
        std::vector<TNT> ret(s, 0.0);
        for (size_t i = 0; i < s ; i++) {
            ret[s - i] = pop_number();
        }
        return ret;
    }
    const char* pop_string() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        auto ret =  data_.back();
        data_.pop_back();
        return ret.as_string();
    }
    const Vec* pop_vector() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        auto ret =  data_.back();
        data_.pop_back();
        return ret.as_vector();
    }
    bool pop_boolean() {
        lupu_assert(data_.size() > 0, "Can't access cell from empty stack!");
        auto ret =  data_.back();
        data_.pop_back();
        return ret.as_boolean();
    }
    void push_number(TNT n) {
        data_.push_back( Cell(n) );
    }
    void push_number_list(std::vector<TNT>& list) {
        for (size_t i = 0; i < list.size(); i++) {
            push_number( list[i] );
        }
        push_number( list.size() );
    }
    void push_vector(Vec* vec) {
        data_.push_back( Cell(vec) );
    }

private:
    void push_string(const char* str) {
        data_.push_back( Cell(str) );
    }

    std::vector< Cell> data_;
};

using HashItem = std::variant<TNT, const char*, Vec>;
struct Hash {
    Hash() {
        target_ = 0;
    }
    ~Hash() {}

    void inc() {
        std::map<const char*, HashItem> new_map;
        maps_.push_back( new_map );
    }

private:
    std::vector< std::map<const char*, HashItem> > maps_;
    size_t target_;
};

struct WordCode {
    enum {
        Number,
        String,
        Builtin,
        Native,
        User,
    } type_;

    std::string str_;
    TNT num_;

    static WordCode new_number(TNT n) {
        WordCode wc;
        wc.type_ = WordCode::Number;
        wc.num_ = n;
        return wc;
    }
    static WordCode new_string(std::string v) {
        WordCode wc;
        wc.type_ = WordCode::String;
        wc.str_ = v;
        return wc;
    }
    static WordCode new_builtin(std::string v) {
        WordCode wc;
        wc.type_ = WordCode::Builtin;
        wc.str_ = v;
        return wc;
    }
    static WordCode new_native(std::string v) {
        WordCode wc;
        wc.type_ = WordCode::Native;
        wc.str_ = v;
        return wc;
    }
    static WordCode new_user(std::string v) {
        WordCode wc;
        wc.type_ = WordCode::User;
        wc.str_ = v;
        return wc;
    }
};

struct WordByte {
    const enum _WordByteType_ {
        Number,
        String,
        GetOperator,
        StaticGetOperator,
        SetOperator,
        StaticSetOperator,
        Native,
        User,
    } type_;

    size_t idx_;
    TNT num_;

    WordByte(_WordByteType_ t, size_t i): type_(t) {
        idx_ = i;
    }
    WordByte(TNT num) : type_(Number) {
        num_ = num;
    }
    WordByte(_WordByteType_ t): type_(t) {
    }
};

// Envrioment
struct Enviroment;
struct Runtime;
struct NativeWord {
    virtual void run(Stack& stack) = 0;
};
using NativeCreator = NativeWord* (Enviroment&);
using UserWord = std::vector<WordCode>;
using UserBinary = std::vector<WordByte>;

struct Enviroment {
    union SettingValue {
        int _int;
        TNT _float;
        bool _bool;
    };

    Enviroment(int sr) {
        SettingValue sv;
        sv._int = sr;
        settings_["SampleRate"] = sv;
    }
    ~Enviroment() {}

    SettingValue query(const std::string& name) {
        if (settings_.find(name) == settings_.end() ) {
            lupu_panic("Can't find target in env's settings!");
        }
        return settings_[name];
    }

    void insert_native_word(const std::string& name, NativeCreator* fn) {
        if ( native_words_.find(name) != native_words_.end() ) {
            lupu_panic("Can't insert native word with same name!");
        }
        native_words_[name] = fn;
    }

    Runtime build(const std::string& txt);

private:
    UserWord compile(const std::string& txt) {
        struct _ {
            static bool parse_number(const std::string& token, TNT& value) {
                if (isdigit(token.at(0)) || (token.at(0) == '-' && token.length() >= 2 && isdigit(token.at(1)))) {
                    if (token.find('.') != std::string::npos || token.find('e') != std::string::npos) { // double
                        value = atof(token.c_str());
                    } else {
                        value = atol(token.c_str());
                    }
                    return true;
                }
                return false;
            }

            static void tokenize_line(std::string const &str_line, std::vector<std::string> &out) {
                std::string state = "SYMBOL";
                std::string current_str = "";

                for (size_t i = 0; i < str_line.size(); i++) {
                    char cur = str_line[i];
                    if ( state == "SYMBOL") {
                        if ( cur == ' ' || cur == '('  || cur == ')' || cur == '{' || cur == '}' ) {
                            // ending of a symbol
                            if (current_str != "") {
                                out.push_back(current_str);
                                current_str = "";
                            }
                            continue;
                        }
                        if ( cur == '[' || cur == ']' ) {
                            // ending of a symbol
                            if (current_str != "") {
                                out.push_back(current_str);
                                current_str = "";
                            }
                            out.push_back( std::string(1, cur) );
                            continue;
                        }

                        if ( cur == '"' ) {
                            if (current_str != "") {
                                lupu_panic("tokenize_line error!");
                            }

                            state = "STRING";
                            current_str.push_back('"');
                            continue;
                        }
                        if ( cur == '\'' ) {
                            if (current_str != "") {
                                lupu_panic("tokenize_line error!");
                            }

                            state = "STRING";
                            current_str.push_back('\'');
                            continue;
                        }

                        if ( cur == ';' ) {
                            if (current_str != "") {
                                out.push_back(current_str);
                            }
                            return;
                        }

                        current_str.push_back(cur);
                        continue;
                    }
                    if ( state == "STRING" ) {
                        if ( cur == '"' && current_str.at(0) == '"') {
                            current_str.push_back('"');
                            out.push_back(current_str);
                            current_str = "";
                            state = "SYMBOL";
                            continue;
                        }
                        if ( cur == '\'' && current_str.at(0) == '\'') {
                            current_str.push_back('\'');
                            out.push_back(current_str);
                            current_str = "";
                            state = "SYMBOL";
                            continue;
                        }
                        current_str.push_back(cur);
                    }
                }
                if ( state == "STRING" ) {
                    lupu_panic("tokenize_line error, string must end in one line!");
                }
                if (current_str != "") {
                    out.push_back(current_str);
                }
            }

            static UserWord loop_macro(UserWord& w) {
                UserWord looped;
                if ( w.size() < 3 ) {
                    lupu_panic("loop macro error, must including iden, begin, end");
                }

                lupu_assert( w[0].type_ == WordCode::String, "first item of loop macro, must be a string!");
                lupu_assert( w[1].type_ == WordCode::Number, "second item of loop macro, must be a begin number!");
                lupu_assert( w[2].type_ == WordCode::Number, "third item of loop macro, must be a end number!");

                auto ident = w[0].str_;
                auto begin = w[1].num_;
                auto end = w[2].num_;
                for ( TNT i = begin; i < end; i = i + 1.0) {
                    for (size_t j = 3; j < w.size(); j++) {
                        if ( w[j].type_ == WordCode::String ) {
                            if ( w[j].str_ == ident ) {
                                looped.push_back( WordCode::new_number( j ) );
                                continue;
                            }
                        }
                        looped.push_back( w[j] );
                    }
                }
                return looped;
            }

            static bool is_valid_name(std::string const &str) {
                if ( str == "true" || str == "false" || str == "null" || str == "@" || str == "@~" || str == "!" || str == "!~" ) {
                    return false;
                }
                if (str.find_first_not_of("_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == std::string::npos) {
                    if ( str.find_first_of("0123456789") != 0 ) {
                        return true;
                    }
                }
                return false;
            }
        };

        // 0. removed comments & tokenize
        std::vector<std::string> tokens;
        std::istringstream code_stream(txt);
        std::string line;
        while (std::getline(code_stream, line)) {
            _::tokenize_line(line,  tokens);
        }

        // 1. tokens post processing
        UserWord main_code;
        std::optional<UserWord> user_code;
        std::optional<UserWord> loop_code;
        std::optional<size_t> list_count;

        for (size_t i = 0; i < tokens.size(); i++) {
            auto token = tokens[i];

            // first pass, processing command primitive
            if ( token == "#def" ) {
                if ( user_code.has_value() ) {
                    lupu_panic("Can't define a new user word inside another user word!");
                }
                if ( loop_code.has_value() ) {
                    lupu_panic("Can't define a new user word inside a loop macro!");
                }
                if ( list_count.has_value() ) {
                    lupu_panic("Can't define a new user word inside a list macro!");
                }
                user_code = UserWord();

                i = i + 1;
                if ( i >= tokens.size() ) {
                    lupu_panic("Can't find #end for #def!");
                }
                token = tokens[i];
                if ( _::is_valid_name(token) ) {
                    if ( user_words_.find(token) == user_words_.end() ) {
                        if ( native_words_.find(token) == native_words_.end() ) {
                            user_code.value().push_back( WordCode::new_string( token) );
                            continue;
                        }
                    }
                }
                lupu_panic("Can't a valid name for #def macro!");

            } else if (token == "#loop" ) {
                if ( list_count.has_value() ) {
                    lupu_panic("Can't define a loop macro inside a list macro!");
                }
                loop_code = UserWord();

                i = i + 1;
                if ( i >= tokens.size() ) {
                    lupu_panic("Can't find #end for #def!");
                }
                token = tokens[i];
                if ( _::is_valid_name(token) ) {
                    if ( user_words_.find(token) == user_words_.end() ) {
                        if ( native_words_.find(token) == native_words_.end() ) {
                            loop_code.value().push_back( WordCode::new_string( token) );
                            continue;
                        }
                    }
                }
                lupu_panic("Can't a valid ident for #loop macro!");
            } else if ( token == "[" ) {
                if ( loop_code.has_value() ) {
                    lupu_panic("Can't define a list macro inside a list macro!");
                }
                list_count = 0;
                continue;
            } else if ( token == "#end" ) {
                if ( list_count.has_value() ) {
                    lupu_panic("Can't ending a word or a loop in a list macro.");
                }

                if ( loop_code.has_value() ) {
                    auto looped = _::loop_macro( loop_code.value() );

                    UserWord& target = main_code;
                    if ( user_code.has_value() ) {
                        target = user_code.value();
                    }
                    for (size_t i = 0; i < looped.size(); i++) {
                        target.push_back(looped[i]);
                    }

                    loop_code.reset();
                    continue;
                }

                if ( user_code.has_value() ) {
                    auto w = user_code.value();
                    if ( w.size() < 1 ) {
                        lupu_panic("define macro error, must including a word name!");
                    }
                    lupu_assert( w[0].type_ == WordCode::String, "first of define macro, must be a name!");

                    auto name = w[0].str_;
                    w.erase(w.begin());
                    user_words_[name] = w;

                    user_code.reset();
                    continue;
                }

                lupu_panic("Find #end without #def or #loop!");
            } else if ( token == "]" ) {
                if ( !list_count.has_value() ) {
                    lupu_panic("']' list macro appears without begin '['!");
                }

                UserWord& target = main_code;
                if ( user_code.has_value() ) {
                    target = user_code.value();
                }

                target.push_back( WordCode::new_number( TNT( list_count.value() ) ) );

                list_count.reset();
                continue;
            }
            // second pass code to number, string,builtin, native, or user
            WordCode newCode;

            if ( token == "ture" ) {
                newCode = WordCode::new_number( 1.0 );
            } else if ( token == "false") {
                newCode = WordCode::new_number( 0.0 );
            } else if ( token == "null" ) {
                newCode = WordCode::new_string( "" );
            } else if ( token == "@" ||
                        token == "@~" ||
                        token == "!" ||
                        token == "!~") {
                newCode = WordCode::new_builtin( token );
            } else if ( token[0] == '"' || token[0] == '\'' || token[0] == '$' ) {
                newCode = WordCode::new_string( token );
            } else if ( _::parse_number(token, newCode.num_) ) {
                newCode = WordCode::new_number( newCode.num_ );
            } else if ( native_words_.find( token ) != native_words_.end() ) {
                newCode = WordCode::new_native( token );
            } else if ( user_words_.find( token ) != user_words_.end() ) {
                newCode = WordCode::new_user( token );
            } else {
                lupu_panic("Find an invalid symbol is not string, number, builtin, user or native!");
            }

            UserWord& target = main_code;
            if ( user_code.has_value() ) {
                target = user_code.value();
            }
            if ( list_count.has_value() ) {
                list_count = list_count.value() + 1;
            }

            target.push_back(newCode);
        }

        if (list_count.has_value()) {
            lupu_panic("List macro without ']' ending!");
        }
        if (loop_code.has_value()) {
            lupu_panic("Loop macro without #end ending!");
        }
        if (user_code.has_value()) {
            lupu_panic("Define macro without #end ending!");
        }

        return main_code;
    }

    NativeWord* create_native(const std::string& name) {
        if ( native_words_.find(name) != native_words_.end() ) {
            return native_words_[name](*this);
        }
        lupu_panic("Call a un registered native word!");
        return nullptr;
    }

    UserWord& get_user(const std::string& name) {
        if ( user_words_.find(name) != user_words_.end() ) {
            return user_words_[name];
        }
        lupu_panic("Call a un registered native word!");
    }

private:
    std::map<std::string, UserWord> user_words_;
    std::map<std::string, NativeCreator*> native_words_;
    std::map<std::string, SettingValue> settings_;

    friend struct Runtime;
};

struct Runtime {
    Runtime() = delete;
    Runtime(Enviroment& env, UserWord& main_code) {
        linking(env, main_code);
    }

private:
    void linking(Enviroment& env, UserWord& word) {
        size_t bin_id = binaries.size();
        binaries.push_back( UserBinary() );

        hash.inc();

        UserBinary bin;
        for(size_t i = 0; i < word.size(); i++) {
            auto code = word[i];
            switch( code.type_ ) {
                case WordCode::Number :
                    bin.push_back( WordByte( code.num_ ) );
                    break;

                case WordCode::String :
                    bin.push_back( WordByte( WordByte::String, string_id( code.str_ ) ) );
                    break;

                case WordCode::Builtin :
                    if ( code.str_ == "@" ) {
                        bin.push_back( WordByte(WordByte::GetOperator) );
                    } else if ( code.str_ == "@~" ) {
                        bin.push_back( WordByte(WordByte::StaticGetOperator) );
                    } else if ( code.str_ == "!" ) {
                        bin.push_back( WordByte(WordByte::SetOperator) );
                    } else if ( code.str_ == "!~" ) {
                        bin.push_back( WordByte(WordByte::StaticSetOperator) );
                    } else {
                        lupu_panic("Find an unsupoorted builtin operator!");
                    }
                    break;

                case WordCode::Native :
                    bin.push_back( WordByte(WordByte::Native, natives.size() ));
                    natives.push_back( env.create_native(code.str_));
                    break;

                case WordCode::User :
                    bin.push_back( WordByte(WordByte::User, binaries.size() ));
                    UserWord& new_word = env.get_user( code.str_ );
                    linking(env, new_word);
                    break;
            }
        }

        binaries[bin_id] = bin;
    }

    size_t string_id(const std::string& str) {
        for (size_t i = 0; i < strings.size(); i++) {
            if ( strings[i] == str ) {
                return i;
            }
        }
        size_t ret = strings.size();
        strings.push_back( str );
        return ret;
    }

private:
    Stack stack;
    Hash hash;

    // resource
    std::vector<std::string> strings;
    std::vector<UserBinary> binaries;
    std::vector<NativeWord*> natives;

    friend struct Enviroment;
};

} // end of namespace
