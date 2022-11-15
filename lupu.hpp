#ifndef _LUPU_H_
#define _LUPU_H_

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


namespace lupu {

// target number type
using Vec = Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic>;
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
    bool is_number() {
        if ( type_ == T_Number ) {
            return true;
        }
        return false;
    }
    bool is_string() {
        if ( type_ == T_String ) {
            return true;
        }
        return false;
    }
    bool is_vector() {
        if ( type_ == T_Vector ) {
            return true;
        }
        return false;
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

    Cell& top() {
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
    void push(Cell cell) {
        data_.push_back(cell);
    }
    void push_string(const char* str) {
        data_.push_back( Cell(str) );
    }

    std::vector< Cell> data_;
    friend struct Runtime;
};

struct Hash {
    using Item = std::variant<TNT, const char*, Vec>;
    Hash() {
        target_ = 0;
    }
    ~Hash() {}

    void inc() {
        std::map<const char*, Item> new_map;
        maps_.push_back( new_map );
    }

    void moveto(size_t i) {
        if ( i >= maps_.size() ) {
            lupu_panic("Can't find target hash!");
        }
        target_ = i;
    }

    Item find(const char* name) {
        if ( maps_[target_].find(name) == maps_[target_].end() ) {
            lupu_panic("Can't find value for name!");
        }
        return maps_[target_][name];
    }

    void set(const char* name, Item item) {
        maps_[target_][name] = std::move(item);
    }

    static Cell Item2Cell( Item* item ) {
        if ( item->index() == 0 ) {
            return Cell( std::get<0>(*item) );
        } else if ( item->index() == 1 ) {
            return Cell( std::get<1>(*item) );
        }
        Vec* vec = & std::get<2>(*item);
        return Cell(vec);
    }

    static Item Cell2Item( Cell& cell ) {
        Item ret;
        if ( cell.type_ == Cell::T_Number ) {
            ret = cell.v._num;
        } else if ( cell.type_ == Cell::T_String ) {
            ret = cell.v._str;
        } else {
            ret = *cell.v._vec;
        }

        return ret;
    }

private:
    std::vector< std::map<const char*, Item> > maps_;
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
        BuiltinOperator,
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
struct BuiltinOperator {
    virtual void run(Stack& stack, Hash& hash) = 0;
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

        load_base_math();
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
    void load_base_math();
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
public:
    Runtime() = delete;
    Runtime(Enviroment& env, UserWord& main_code) {
        linking(env, main_code);
    }
    void run() {
        run_(0);
    }
private:
    void run_(size_t from) {
        hash.moveto(from);
        for ( size_t i = 0; i < binaries[from].size(); i++) {
            auto byte = binaries[from][i];
            switch( byte.type_ ) {
                case WordByte::Number:
                    stack.push_number( byte.num_ );
                    break;

                case WordByte::String:
                    {
                        const char* str = strings[ byte.idx_ ].c_str();
                        stack.push_string(str);
                    }
                    break;

                case WordByte::BuiltinOperator:
                    builtins[ byte.idx_ ]->run( stack, hash );
                    break;

                case WordByte::Native:
                    natives[ byte.idx_ ]->run( stack );
                    break;

                case WordByte::User:
                    run_(from + 1);
                    break;
            }
        }
    }

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
                    {
                        BuiltinOperator* op = nullptr;
                        if ( code.str_ == "@" ) {
                            op = new BuiltinGet();
                        } else if ( code.str_ == "@~" ) {
                            op = new BuiltinStaticGet();
                        } else if ( code.str_ == "!" ) {
                            op = new BuiltinSet();
                        } else if ( code.str_ == "!~" ) {
                            op = new BuiltinStaticSet();
                        } else {
                            lupu_panic("Find an unsupoorted builtin operator!");
                        }
                        size_t idx = builtins.size();
                        builtins.push_back(op);
                        bin.push_back( WordByte( WordByte::BuiltinOperator, idx) );
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

    struct BuiltinGet : public BuiltinOperator {
        Hash::Item value;
        virtual void run(Stack& stack, Hash& hash) {
            const char* name = stack.pop_string();
            value = hash.find(name);
            stack.push( Hash::Item2Cell(&value) );
        }
    };

    struct BuiltinStaticGet : public BuiltinOperator {
        Hash::Item value;
        bool first;
        BuiltinStaticGet() {
            first = false;
        }
        virtual void run(Stack& stack, Hash& hash) {
            if ( first == false) {
                first = true;
                const char* name = stack.pop_string();
                value = hash.find(name);
                stack.push( Hash::Item2Cell(&value) );
            }
            stack.pop_string();
            stack.push( Hash::Item2Cell(&value) );
        }
    };

    struct BuiltinSet : public BuiltinOperator {
        virtual void run(Stack& stack, Hash& hash) {
            const char* name = stack.pop_string();
            Cell cell = stack.pop();
            hash.set(name, Hash::Cell2Item(cell));
        }
    };

    struct BuiltinStaticSet : public BuiltinOperator {
        bool first;
        BuiltinStaticSet() {
            first = false;
        }
        virtual void run(Stack& stack, Hash& hash) {
            if ( first == false) {
                first = true;
                const char* name = stack.pop_string();
                Cell cell = stack.pop();
                hash.set(name, Hash::Cell2Item(cell));
                return;
            }
            stack.pop_string();
            stack.pop();
        }
    };


private:
    Stack stack;
    Hash hash;

    // resource
    std::vector<std::string> strings;
    std::vector<UserBinary> binaries;
    std::vector<NativeWord*> natives;
    std::vector<BuiltinOperator*> builtins;

    friend struct Enviroment;
};



struct StaticNativeWord : public NativeWord {
    StaticNativeWord() {
        first = false;
    }
    virtual void run(Stack& stack) {
        if ( first == false ) {
            first = true;
            run_first(stack);
            return;
        }
        run_next(stack);
    }
    virtual void run_first(Stack& stack) = 0;
    virtual void run_next(Stack& stack) = 0;
private:
    bool first;
};

#define NWORD_CREATOR_DEFINE_LUPU(CLS)         \
static NativeWord* creator(Enviroment& env) {   \
    NativeWord* wd = new CLS();                \
    return wd;                                 \
}
namespace base {
    struct Drop : public NativeWord {
        virtual void run(Stack& stack) {
            stack.drop();
        }
        NWORD_CREATOR_DEFINE_LUPU(Drop)
    };

    struct Dup : public NativeWord {
        virtual void run(Stack& stack) {
            stack.dup();
        }
        NWORD_CREATOR_DEFINE_LUPU(Dup)
    };

    struct Dup2 : public NativeWord {
        virtual void run(Stack& stack) {
            stack.dup2();
        }
        NWORD_CREATOR_DEFINE_LUPU(Dup2)
    };

    struct Swap : public NativeWord {
        virtual void run(Stack& stack) {
            stack.swap();
        }
        NWORD_CREATOR_DEFINE_LUPU(Swap)
    };

    struct Rot : public NativeWord {
        virtual void run(Stack& stack) {
            stack.rot();
        }
        NWORD_CREATOR_DEFINE_LUPU(Rot)
    };

    struct Zeros : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            size_t len = stack.pop_number();
            vec = Vec::Zero(len, 1);
            stack.push_vector( &vec);
        }
        virtual void run_next(Stack& stack) {
            stack.pop_number();
            stack.push_vector( &vec);
        }
        NWORD_CREATOR_DEFINE_LUPU(Zeros)
    private:
        Vec vec;
    };

    struct Ones : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            size_t len = stack.pop_number();
            vec = Vec::Ones(len, 1);
            stack.push_vector( &vec);
        }
        virtual void run_next(Stack& stack) {
            stack.pop_number();
            stack.push_vector( &vec);
        }
        NWORD_CREATOR_DEFINE_LUPU(Ones)
    private:
        Vec vec;
    };

    struct Randoms : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            size_t len = stack.pop_number();
            vec = Vec::Random(len, 1);
            stack.push_vector( &vec);
        }
        virtual void run_next(Stack& stack) {
            stack.pop_number();
            stack.push_vector( &vec);
        }
        NWORD_CREATOR_DEFINE_LUPU(Randoms)
    private:
        Vec vec;
    };

    struct Matrix : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            size_t col = stack.pop_number();
            size_t row = stack.pop_number();
            vec = Vec::Zero(col, row);
            stack.push_vector( &vec);
        }
        virtual void run_next(Stack& stack) {
            stack.pop_number();
            stack.push_vector( &vec);
        }
        NWORD_CREATOR_DEFINE_LUPU(Matrix)
    private:
        Vec vec;
    };

}

#define BIN_OP_MATH_WORD_LUPU(CLS, op)              \
struct CLS : public NativeWord {                    \
    virtual void run(Stack& stack) {                \
        if ( stack.top().is_number() ) {            \
            auto a = stack.pop_number();            \
            auto b = stack.pop_number();            \
            stack.push_number( a op b);             \
            return;                                 \
        } else if ( stack.top().is_vector() ) {     \
            auto a = stack.pop_vector();            \
            if ( stack.top().is_number() ) {        \
                auto b = stack.pop_number();        \
                result = *a op b;                   \
                stack.push_vector(&result);         \
                return;                             \
            } else if ( stack.top().is_vector() ) { \
                auto b = stack.pop_vector();        \
                result = *a op *b;                  \
                stack.push_vector(&result);         \
                return;                             \
            }                                       \
        }                                           \
        lupu_panic("#CLS don't support type!");     \
    }                                               \
    static NativeWord* creator(Enviroment& env) {   \
         NativeWord* wd = new CLS();                \
         return wd;                                 \
    }                                               \
private:                                            \
    Vec result;                                     \
}

#define UNI_MATH_WORD_LUPU(CLS, op)                 \
struct CLS : public NativeWord {                    \
    virtual void run(Stack& stack) {                \
        if ( stack.top().is_number() ) {            \
            auto a = stack.pop_number();            \
            stack.push_number( std::op(a) );        \
            return;                                 \
        } else if ( stack.top().is_vector() ) {     \
            auto a = stack.pop_vector();            \
            result = a->op();                       \
            return;                                 \
        }                                           \
        lupu_panic("#CLS don't support type!");     \
    }                                               \
    static NativeWord* creator(Enviroment& env) {   \
         NativeWord* wd = new CLS();                \
         return wd;                                 \
    }                                               \
private:                                            \
    Vec result;                                     \
}


namespace math {
    BIN_OP_MATH_WORD_LUPU(Add, +);
    BIN_OP_MATH_WORD_LUPU(Sub, -);
    BIN_OP_MATH_WORD_LUPU(Mul, *);
    BIN_OP_MATH_WORD_LUPU(Div, /);

    UNI_MATH_WORD_LUPU(Abs, abs);
    UNI_MATH_WORD_LUPU(Arg, arg);
    UNI_MATH_WORD_LUPU(Exp, exp);
    UNI_MATH_WORD_LUPU(Log, log);
    UNI_MATH_WORD_LUPU(Log1p, log1p);
    UNI_MATH_WORD_LUPU(Log10, log10);
    UNI_MATH_WORD_LUPU(Sqrt, sqrt);

    UNI_MATH_WORD_LUPU(Sin, sin);
    UNI_MATH_WORD_LUPU(Cos, cos);
    UNI_MATH_WORD_LUPU(Tan, tan);
    UNI_MATH_WORD_LUPU(Asin, asin);
    UNI_MATH_WORD_LUPU(Acos, acos);
    UNI_MATH_WORD_LUPU(Atan, atan);

    UNI_MATH_WORD_LUPU(Sinh, sinh);
    UNI_MATH_WORD_LUPU(Cosh, cosh);
    UNI_MATH_WORD_LUPU(Tanh, tanh);
    UNI_MATH_WORD_LUPU(Asinh, asinh);
    UNI_MATH_WORD_LUPU(Acosh, acosh);
    UNI_MATH_WORD_LUPU(Atanh, atanh);

    UNI_MATH_WORD_LUPU(Ceil, ceil);
    UNI_MATH_WORD_LUPU(Floor, floor);
    UNI_MATH_WORD_LUPU(Round, round);

    struct Mod : public NativeWord {
        virtual void run(Stack& stack) {
            auto a = stack.pop_number();
            auto b = stack.pop_number();
            stack.push_number( fmod(a, b) );
        }
        NWORD_CREATOR_DEFINE_LUPU(Mod)
    };

    struct Inv : public NativeWord {
        virtual void run(Stack& stack) {
            if ( stack.top().is_number() ) {
                auto a = stack.pop_number();
                stack.push_number( 1.0 / a );
                return;
            }
            auto a = stack.pop_vector();
            result = a->inverse();
            stack.push_vector(&result);
        }
        NWORD_CREATOR_DEFINE_LUPU(Inv);
    private:
        Vec result;
    };

    struct Pow : public NativeWord {
        virtual void run(Stack& stack) {
            if ( stack.top().is_number() ) {
                auto a = stack.pop_number();
                auto b = stack.pop_number();
                stack.push_number( std::pow(a, b) );
                return;
            }
            auto a = stack.pop_vector();
            auto b = stack.pop_vector();
            result = a->pow(*b);
            stack.push_vector(&result);
        }
        NWORD_CREATOR_DEFINE_LUPU(Pow)
    private:
        Vec result;
    };

}


void Enviroment::load_base_math() {
    // base words
    insert_native_word("drop", base::Drop::creator );
    insert_native_word("dup", base::Dup::creator );
    insert_native_word("dup2", base::Dup2::creator );
    insert_native_word("swap", base::Swap::creator );
    insert_native_word("rot", base::Rot::creator );

    insert_native_word("zeros~", base::Zeros::creator );
    insert_native_word("ones~", base::Ones::creator );
    insert_native_word("randoms~", base::Randoms::creator );
    insert_native_word("matrix~", base::Matrix::creator );

    // math words
    insert_native_word("+", math::Add::creator );
    insert_native_word("-", math::Sub::creator );
    insert_native_word("*", math::Mul::creator );
    insert_native_word("/", math::Div::creator );
    insert_native_word("%", math::Mod::creator );

    insert_native_word("abs", math::Abs::creator );
    insert_native_word("arg", math::Arg::creator );
    insert_native_word("exp", math::Exp::creator );
    insert_native_word("inv", math::Inv::creator );
    insert_native_word("log", math::Log::creator );
    insert_native_word("log1p", math::Log1p::creator );
    insert_native_word("log10", math::Log10::creator );
    insert_native_word("pow", math::Pow::creator );

    insert_native_word("sin", math::Sin::creator );
    insert_native_word("cos", math::Cos::creator );
    insert_native_word("tan", math::Tan::creator );
    insert_native_word("asin", math::Asin::creator );
    insert_native_word("acos", math::Acos::creator );
    insert_native_word("atan", math::Atan::creator );

    insert_native_word("sinh", math::Sinh::creator );
    insert_native_word("cosh", math::Cosh::creator );
    insert_native_word("tanh", math::Tanh::creator );
    insert_native_word("asinh", math::Asinh::creator );
    insert_native_word("acosh", math::Acosh::creator );
    insert_native_word("atanh", math::Atanh::creator );

    insert_native_word("ceil", math::Ceil::creator );
    insert_native_word("floor", math::Floor::creator );
    insert_native_word("round", math::Round::creator );
}

Runtime Enviroment::build(const std::string& txt) {
    auto main_code = compile(txt);
    Runtime rt(*this, main_code);
    return rt;
}

} // end of namespace
#endif
