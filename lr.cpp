#include "lr.hpp"

namespace lr {

std::ostream& operator<<(std::ostream& os, const Cell& c) {
    if ( c.type_ == Cell::T_String ) {
        os << "S:" << c.v._str;
    } else if ( c.type_ == Cell::T_Number ) {
        os << "N:" << c.v._num;
    } else {
        os << "V: (" << std::endl << *(c.v._vec) << " )";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const WordCode& c) {
    if ( c.type_ == WordCode::String ) {
        os << "S:" << c.str_;
    } else if ( c.type_ == WordCode::Number ) {
        os << "N:" << c.num_;
    } else if ( c.type_ == WordCode::Builtin ) {
        os << "B:" << c.str_;
    } else if ( c.type_ == WordCode::Native ) {
        os << "NA:" << c.str_;
    } else {
        os << "U:" << c.str_;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, Stack& stack) {
    os << "----STACK(" << stack.size() << ")----" << std::endl;
    for (size_t i = 0; i < stack.data_.size(); i++) {
        os << "==>" << i << " " << stack.data_[i] << std::endl;
    }
    os << "----" << std::endl;
    return os;
}

namespace base {
    struct Exit : public NativeWord {
        virtual void run(Stack& stack) {
            exit(0);
        }
        NWORD_CREATOR_DEFINE_LR(Exit)
    };

    struct Dump : public NativeWord {
        virtual void run(Stack& stack) {
            std::cout << stack << std::endl;
        }
        NWORD_CREATOR_DEFINE_LR(Dump)
    };

    struct Drop : public NativeWord {
        virtual void run(Stack& stack) {
            stack.drop();
        }
        NWORD_CREATOR_DEFINE_LR(Drop)
    };

    struct Dup : public NativeWord {
        virtual void run(Stack& stack) {
            stack.dup();
        }
        NWORD_CREATOR_DEFINE_LR(Dup)
    };

    struct Dup2 : public NativeWord {
        virtual void run(Stack& stack) {
            stack.dup2();
        }
        NWORD_CREATOR_DEFINE_LR(Dup2)
    };

    struct Swap : public NativeWord {
        virtual void run(Stack& stack) {
            stack.swap();
        }
        NWORD_CREATOR_DEFINE_LR(Swap)
    };

    struct Rot : public NativeWord {
        virtual void run(Stack& stack) {
            stack.rot();
        }
        NWORD_CREATOR_DEFINE_LR(Rot)
    };

    struct OnlyOnce : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            return;
        }
        virtual void run_next(Stack& stack) {
            stack.drop();
        }
        NWORD_CREATOR_DEFINE_LR(OnlyOnce)
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
        NWORD_CREATOR_DEFINE_LR(Zeros)
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
        NWORD_CREATOR_DEFINE_LR(Ones)
    private:
        Vec vec;
    };

    struct Numbers : public StaticNativeWord {
        virtual void run_first(Stack& stack) {
            size_t len = stack.pop_number();
            TNT n = stack.pop_number();
            vec = Vec::Ones(len, 1) * n;
            stack.push_vector( &vec);
        }
        virtual void run_next(Stack& stack) {
            stack.pop_number();
            stack.pop_number();
            stack.push_vector( &vec);
        }
        NWORD_CREATOR_DEFINE_LR(Numbers)
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
        NWORD_CREATOR_DEFINE_LR(Randoms)
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
        NWORD_CREATOR_DEFINE_LR(Matrix)
    private:
        Vec vec;
    };

}

#define BIN_OP_MATH_WORD_LR(CLS, op)              \
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
        lr_panic("#CLS don't support type!");     \
    }                                               \
    static NativeWord* creator(Enviroment& env) {   \
         NativeWord* wd = new CLS();                \
         return wd;                                 \
    }                                               \
private:                                            \
    Vec result;                                     \
}

#define UNI_MATH_WORD_LR(CLS, op)                 \
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
        lr_panic("#CLS don't support type!");     \
    }                                               \
    static NativeWord* creator(Enviroment& env) {   \
         NativeWord* wd = new CLS();                \
         return wd;                                 \
    }                                               \
private:                                            \
    Vec result;                                     \
}


namespace math {
    BIN_OP_MATH_WORD_LR(Add, +);
    BIN_OP_MATH_WORD_LR(Sub, -);
    BIN_OP_MATH_WORD_LR(Mul, *);
    BIN_OP_MATH_WORD_LR(Div, /);

    UNI_MATH_WORD_LR(Abs, abs);
    UNI_MATH_WORD_LR(Arg, arg);
    UNI_MATH_WORD_LR(Exp, exp);
    UNI_MATH_WORD_LR(Log, log);
    UNI_MATH_WORD_LR(Log1p, log1p);
    UNI_MATH_WORD_LR(Log10, log10);
    UNI_MATH_WORD_LR(Sqrt, sqrt);

    UNI_MATH_WORD_LR(Sin, sin);
    UNI_MATH_WORD_LR(Cos, cos);
    UNI_MATH_WORD_LR(Tan, tan);
    UNI_MATH_WORD_LR(Asin, asin);
    UNI_MATH_WORD_LR(Acos, acos);
    UNI_MATH_WORD_LR(Atan, atan);

    UNI_MATH_WORD_LR(Sinh, sinh);
    UNI_MATH_WORD_LR(Cosh, cosh);
    UNI_MATH_WORD_LR(Tanh, tanh);
    UNI_MATH_WORD_LR(Asinh, asinh);
    UNI_MATH_WORD_LR(Acosh, acosh);
    UNI_MATH_WORD_LR(Atanh, atanh);

    UNI_MATH_WORD_LR(Ceil, ceil);
    UNI_MATH_WORD_LR(Floor, floor);
    UNI_MATH_WORD_LR(Round, round);

    struct Mod : public NativeWord {
        virtual void run(Stack& stack) {
            auto a = stack.pop_number();
            auto b = stack.pop_number();
            stack.push_number( fmod(a, b) );
        }
        NWORD_CREATOR_DEFINE_LR(Mod)
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
        NWORD_CREATOR_DEFINE_LR(Inv);
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
        NWORD_CREATOR_DEFINE_LR(Pow)
    private:
        Vec result;
    };

    struct PI : public NativeWord {
        virtual void run(Stack& stack) {
            stack.push_number(M_PI);
        }
        NWORD_CREATOR_DEFINE_LR(PI);
    };

    struct E : public NativeWord {
        virtual void run(Stack& stack) {
            stack.push_number(M_E);
        }
        NWORD_CREATOR_DEFINE_LR(E);
    };
}

void Enviroment::load_base_math() {
    // base words
    insert_native_word("drop", base::Drop::creator );
    insert_native_word("dup", base::Dup::creator );
    insert_native_word("dup2", base::Dup2::creator );
    insert_native_word("swap", base::Swap::creator );
    insert_native_word("rot", base::Rot::creator );
    insert_native_word("~", base::OnlyOnce::creator);
    insert_native_word("?", base::Dump::creator );
    insert_native_word("^", base::Exit::creator );

    insert_native_word("zeros~", base::Zeros::creator );
    insert_native_word("ones~", base::Ones::creator );
    insert_native_word("numbers~", base::Numbers::creator );
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

    insert_native_word("math.pi", math::PI::creator );
    insert_native_word("math.e", math::E::creator );
}

Runtime Enviroment::build(const std::string& txt) {
    auto main_code = compile(txt);
    Runtime rt(*this, main_code);
    return rt;
}


}
