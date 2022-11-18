#include <fstream>

#include "viz_visitor.hxx"

namespace bassoon
{
namespace viz
{

void VizVisitor::visualiseAST(std::shared_ptr<NodeAST> AST){
    std::ofstream output("AST_Trees.dot", std::ofstream::out);
    output << "digraph { \n";
        output << "A->B";
    output << "}";
    output.close();
}

void VizVisitor::visualiseASTs(std::vector<std::shared_ptr<NodeAST>> ASTs){
    
}
    
} // namespace viz

} // namespace bassoon
