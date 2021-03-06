#include "closure.h"

#include "astexpr.h"
#include "stream.h"

Stream *Closure::execute(Object *in)
{
    Context *new_context = new Context(context);

    Stream *add = new Stream[exprs.size()];
    Stream *add_end = add + exprs.size();

    while (add < add_end)
    {
        new_context->push_back(add);
        add++;
    }

    Stream *res = new Stream();

    std::vector<AstExpr*>::iterator i = exprs.begin();
    while (i != exprs.end())
    {
        (*i)->execute(new_context)->set_flows_to(res);
        i++;
    }

    return res;
}

std::string Closure::to_string()
{
    return "[ Closure ]";
}
