#include "astblock_old.h"

#include "astident.h"
#include "astflow.h"
#include "closure.h"
#include "stream.h"

/*
AstBlock::BindFlag operator|(AstFunction::BindFlag a, AstFunction::BindFlag b)
{ return static_cast<AstFunction::BindFlag>(static_cast<int>(a) | static_cast<int>(b)); }

AstBlock::BindFlag operator&(AstFunction::BindFlag a, AstFunction::BindFlag b)
{ return static_cast<AstFunction::BindFlag>(static_cast<int>(a) & static_cast<int>(b)); }

AstBlock::BindFlag operator^(AstFunction::BindFlag a, AstFunction::BindFlag b)
{ return static_cast<AstFunction::BindFlag>(static_cast<int>(a) ^ static_cast<int>(b)); }

AstBlock::BindFlag operator~(AstFunction::BindFlag a)
{ return static_cast<AstFunction::BindFlag>(~static_cast<int>(a)); }

AstBlock::BindFlag operator|=(AstFunction::BindFlag &a, AstFunction::BindFlag b)
{ return a = static_cast<AstFunction::BindFlag>(static_cast<int>(a) | static_cast<int>(b)); }

AstBlock::BindFlag operator&=(AstFunction::BindFlag &a, AstFunction::BindFlag b)
{ return a = static_cast<AstFunction::BindFlag>(static_cast<int>(a) & static_cast<int>(b)); }

AstBlock::BindFlag operator^=(AstFunction::BindFlag &a, AstFunction::BindFlag b)
{ return a = static_cast<AstFunction::BindFlag>(static_cast<int>(a) ^ static_cast<int>(b)); }
*/

void AstBlock::set_bind(const Flags flags)
{
    bind |= flags;
}

void AstBlock::apply_bind(AstBlock *scope)
{
    parent_scope = scope;
    scope = this;

    if (bind & BindInOut)
    {
        declared_symbols.push_back("in");
        declared_symbols.push_back("out");
    }

    if (bind & BindImplicitRel)
    {
        declared_symbols.push_back(AstIdent::ImplicitIn);
        declared_symbols.push_back(AstIdent::ImplicitOut);
    }

    if (bind & BindAutoOut)
    {
        declared_symbols.push_back(AstIdent::AutoOut);

        std::vector<AstExpr*>::iterator i = exprs.begin();
        while (i != exprs.end())
        {
            *i = new AstFlow(new AstIdent(AstIdent::AutoOut), *i);
            i++;
        }
    }

    bind_ident_decl = bind & BindIdentDecl;

    std::vector<AstExpr*>::iterator i = exprs.begin();
    while (i != exprs.end())
    {
        (*i)->apply_bind(scope);
        i++;
    }
}

void AstBlock::set_stack_start(unsigned int stack_size)
{
    stack_start = stack_size;
    stack_size += declared_symbols.size();
}

unsigned int AstBlock::hoist_ident(std::string symbol, bool declare)
{
    unsigned int index = static_cast<unsigned int>(-1);

    std::vector<std::string>::iterator i = declared_symbols.begin();
    while (i != declared_symbols.end())
    {
        if (*i == symbol)
        {
            index = i - declared_symbols.begin();
            break;
        }
        i++;
    }

    if (declare)
    {
        if (bind_ident_decl)
        {
            if (index == -1)
            {
                index = declared_symbols.size();
                declared_symbols.push_back(symbol);
            }
            else
            {
                std::cerr << "Identifier \"" << symbol << "\" is declared twice in this scope" << std::endl;
            }
        }
        else
        {
            if (index != -1)
            {
                std::cerr << "Identifier \"" << symbol << "\" is shadowed by another declaration of the same name" << std::endl;
                index = -1;
            }
        }
    }

    if (index != -1)
    {
        return stack_start + index;
    }
    else
    {
        if (parent_scope)
        {
            return parent_scope->hoist_ident(symbol, declare);
        }
        else
        {
            std::cerr << "Identifier \"" << symbol << "\" is not declared in this scope" << std::endl;
            return 0;
        }
    }
}


Stream *AstBlock::execute(Context *context)
{
    Closure *closure = new Closure(exprs, declared_symbols.size(), context);

    Stream *res = new Stream();
    res->flow_from(closure);
    return res;
}
