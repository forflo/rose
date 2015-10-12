#ifndef ROSE_BinaryAnalysis_SymbolicExprParser_H
#define ROSE_BinaryAnalysis_SymbolicExprParser_H

#include <BinarySymbolicExpr.h>
#include <Sawyer/SharedPointer.h>

namespace rose {
namespace BinaryAnalysis {

/** Parses symbolic expressions from text.
 *
 *  Parses symbolic expressions using a simple LISP-like syntax, the same syntax used by the unparser.  An expression can be a
 *  numeric constant with an optional negative sign, a hexadecimal bit vector, a numbered variable like "v123", a named
 *  variable, or an operator with arguments.  Operators are expressed as an operator name and space-separated arguments; the
 *  operator and arguments are both inside the parentheses like in LISP.  A width in bits can be appended to any constant,
 *  variable, or operator name and is a positive number inside square brackets.  Inline comments are enclosed in angle brackets
 *  (less than and greater than signs) and can be nested. Backslashes are used to escape parentheses, square brackets, and
 *  angle brackets to remove their special meaning. The normal C/C++ backslash escapes are also understood.
 *
 *  @todo Symbolic expressions were originally intended to be used only within ROSE and therefore many of the
 *        operations assert that their arguments are correct. Now that users can easily construct their own symbolic
 *        expressions from text, we need to make the failure modes less extreme. [Robb P. Matzke 2015-09-22]. */
class SymbolicExprParser {
public:
    /** Syntax errors that are thrown. */
    class SyntaxError: public std::runtime_error {
    public:
        std::string inputName;                          /**< Name of input, usually a file name. */
        unsigned lineNumber;                            /**< Line number (1-origin). Zero if position info is not available. */
        unsigned columnNumber;                          /**< Column number (0-origin). Unused when @c lineNumber is zero. */
        void print(std::ostream&) const;                /**< Print error message to stream. */
        SyntaxError(const std::string &mesg, const std::string &inputName, unsigned lineNumber, unsigned columnNumber);
        ~SyntaxError() throw () {}
    };

    /** Tokens generated by the lexical analysis. */
    class Token {
    public:
        /** Token types. */
        enum Type {
            NONE = 0,                                   /**< End of input. */
            LTPAREN,                                    /**< Left parenthesis. */
            RTPAREN,                                    /**< Right parenthesis. */
            BITVECTOR,                                  /**< Numeric constant. */
            SYMBOL                                      /**< Symbol. Symbols are basically anything but white space, comments,
                                                         *   and the other token types. */
        };

    private:
        Type type_;
        std::string lexeme_;                            // lexeme
        size_t width_;                                  // width of value in bits
        Sawyer::Container::BitVector bits_;             // bits representing constant terms
        unsigned lineNumber_, columnNumber_;            // for start of token

    public:
        /** Constructs an end-of-input token with no position information. */
        Token()
            : type_(NONE), width_(0), lineNumber_(0), columnNumber_(0) {}

        /** Constructs a specific token from a string. Do not use this to construct numeric tokens. */
        Token(Type type, size_t width, const std::string &lexeme, unsigned lineNumber, unsigned columnNumber)
            : type_(type), lexeme_(lexeme), width_(width), lineNumber_(lineNumber), columnNumber_(columnNumber) {
            ASSERT_forbid(BITVECTOR==type);
        }

        /** Construct a token for a numeric constant. */
        Token(const Sawyer::Container::BitVector &bv, const std::string &lexeme, unsigned lineNumber, unsigned columnNumber)
            : type_(BITVECTOR), lexeme_(lexeme), width_(bv.size()), bits_(bv),
              lineNumber_(lineNumber), columnNumber_(columnNumber) {}

        /** Creates a syntax error from a token plus message. */
        SymbolicExprParser::SyntaxError syntaxError(const std::string &mesg, const std::string &name="input") const {
            return SymbolicExprParser::SyntaxError(mesg, name, lineNumber_, columnNumber_);
        }

        /** Token type. */
        Type type() const { return type_; }

        /** Lexeme from which token was parsed. */
        const std::string &lexeme() const { return lexeme_; }

        /** Width of expression in bits. */
        size_t width() const { return width_; }

        /** Bit vector for numeric constants. The bit vector will be empty for non-numeric tokens. */
        const Sawyer::Container::BitVector& bits() const { return bits_; }

        /** Line number for start of token. */
        unsigned lineNumber() const { return lineNumber_; }

        /** Column number for start of token. */
        unsigned columnNumber() const { return columnNumber_; }
    };

    /** A stream of tokens.
     *
     *  This container is conceptually infinite size padded with EOF tokens, although in practice it usually holds only one or
     *  two tokens at a time.  Tokens are consumed by shifting them off the beginning of the stream. */
    class TokenStream {
        std::istream &input_;
        std::string name_;
        unsigned lineNumber_, columnNumber_;
        const Token endToken_;
        std::vector<Token> tokens_;
        int readAhead_;

    public:
        /** Scan tokens from a character stream.
         *
         *  The @p name parameter is the name to use for this stream in error messages (usually a file name), and the @p
         *  lineNumber and @p columnNumber are the position of the first character in the stream. */
        explicit TokenStream(std::istream &input, const std::string &name="input",
                             unsigned lineNumber=1, unsigned columnNumber=0)
            : input_(input), name_(name), lineNumber_(lineNumber), columnNumber_(columnNumber), readAhead_(EOF) {
            init();
        }

        /** Name of this input stream. */
        const std::string& name() const { return name_; }

        /** Current line number. */
        unsigned lineNumber() const { return lineNumber_; }

        /** Current column number. */
        unsigned columnNumber() const { return columnNumber_; }

        /** Returns the specified token without consuming it. */
        const Token& operator[](size_t idx);

        /** Consume the specified number of tokens. */
        void shift(size_t n=1);

        /** Returns the next character.  Returns the next character of input or EOF without consuming it. */
        int nextCharacter();

        /** Consume the next character. Advances over the next character, adjusting line and column information.  Returns the
         * character consumed, or EOF. */
        int consumeCharacter();

        /** Skip over characters until a non-white-space character is encountered. */
        void consumeWhiteSpace();

        /** Skip over an escape sequence and return the escaped character. */
        int consumeEscapeSequence();

        /** Skip over angle-bracket comments. Consumes angle bracket comments like "<this is a comment>". Any right angle
         *  brackets that are part of the comment must be escaped. Unescaped angle brackets can be nested like parentheses. */
        void consumeInlineComment();

        /** Skip over white space and/or inline comments. */
        void consumeWhiteSpaceAndComments();

        /** Parse and consume a term.
         *
         *  A term is a numeric constant or a symbol.  Symbols that start with a letter or underscore contain only letters,
         *  underscores, and decimal digits.  Symbols that start with other than a letter or underscore end at the first
         *  unescaped white-space, comment, parenthesis, angle bracket, or square bracket. Escaping the special character with
         *  a backslash will cause it to become part of the symbol. */
        std::string consumeTerm();

        /** Parse and consume a width specification. A width specification is a decimal number in square brackets. */
        size_t consumeWidth();

        /** Parse and consume the next token. Parses and consumes the next token and return it. Returns the special NONE token
         * at end-of-input. */
        Token scan();

    private:
        void init();

        // Try to fill the token vector so it contains tokens up through at least [idx]
        void fillTokenList(size_t idx);
    };

    /** Virtual base class for atom and operator expansion. */
    class Expansion: public Sawyer::SharedObject {
        std::string title_;
        std::string docString_;
    public:
        virtual ~Expansion() {}

        /** Shared pointer. Uses reference counting. */
        typedef Sawyer::SharedPointer<Expansion> Ptr;

        /** Property: Title to use for documentation.
         *
         * @{ */
        const std::string& title() const { return title_; }
        void title(const std::string &s) { title_ = s; }
        /** @} */

        /** Property: Documentation string.
         *
         *  The string uses the simple markup language from Sawyer's command-line processing.
         *
         * @{ */
        const std::string& docString() const { return docString_; }
        void docString(const std::string &s) { docString_ = s; }
        /** @} */
    };

    /** Virtual base class for expanding atoms. */
    class AtomExpansion: public Expansion {
    public:
        /** Shared pointer. Uses reference counting. */
        typedef Sawyer::SharedPointer<AtomExpansion> Ptr;

        /** Operator to expand the symbol into an expression tree. The width in bits is either the width specified in square
         *  brackets for the symbol, or zero. Functors are all called for each symbol, and the first one to return non-null is
         *  the one that's used to generate the symbolic expression. */
        virtual SymbolicExpr::Ptr operator()(const Token &name) = 0;
    };

    /** Virtual base class for expanding operators. */
    class OperatorExpansion: public Expansion {
    public:
        virtual ~OperatorExpansion() {}

        /** Shared pointer. Uses reference counting. */
        typedef Sawyer::SharedPointer<OperatorExpansion> Ptr;

        /** Operator to expand a list into an expression tree. The width in bits is either the width specified in
         *  square brackets for the function symbol, or zero.  Functors are all called for each symbol, and the first one to
         *  return non-null is the one that's used to generate the symbolic expression. */
        virtual SymbolicExpr::Ptr operator()(const Token &name, const SymbolicExpr::Nodes &operands) = 0;
    };

    /** Ordered atom table. */
    typedef std::vector<AtomExpansion::Ptr> AtomTable;

    /** Ordered operator table. */
    typedef std::vector<OperatorExpansion::Ptr> OperatorTable;

private:
    AtomTable atomTable_;
    OperatorTable operatorTable_;

public:
    /** Default constructor. */
    SymbolicExprParser() { init(); }

    /** Create a symbolic expression by parsing a string.
     *
     *  Parses the string and returns the first expression in the string. Throws a @ref SyntaxError if problems are
     *  encountered. */
    SymbolicExpr::Ptr parse(const std::string&, const std::string &inputName="string");

    /** Create a symbolic expression by parsing a file.
     *
     *  Parses the file and returns the first expression in the file. Throws a @ref SyntaxError if problems are encountered. */
    SymbolicExpr::Ptr parse(std::istream &input, const std::string &filename,
                                    unsigned lineNumber=1, unsigned columnNumber=0);

    /** Create a symbolic expression by parsing a token stream.
     *
     *  Parses the token stream and returns its first expression. Throws a @ref SyntaxError if problems are encountered. */
    SymbolicExpr::Ptr parse(TokenStream&);

    /** Append a new functor for expanding atoms into symbolic expressions. */
    void appendAtomExpansion(const AtomExpansion::Ptr&);

    /** Append a new functor for expanding operators into symbolic expressions. */
    void appendOperatorExpansion(const OperatorExpansion::Ptr&);

    /** Return all atom expansion functors.
     *
     * @{ */
    const AtomTable& atomTable() const { return atomTable_; }
    AtomTable& atomTable() { return atomTable_; }
    /** @} */

    /** Return all operator expansion functors.
     *
     * @{ */
    const OperatorTable& operatorTable() const { return operatorTable_; }
    OperatorTable& operatorTable() { return operatorTable_; }
    /** @} */

    /** Documentation string.
     *
     *  Returns the documentation string for this parser. The documentation string is a a simple markup language that can be
     *  used by command-line parsers. */
    std::string docString() const;

private:
    void init();
};
    
std::ostream& operator<<(std::ostream&, const SymbolicExprParser::SyntaxError&);

} // namespace
} // namespace

#endif
