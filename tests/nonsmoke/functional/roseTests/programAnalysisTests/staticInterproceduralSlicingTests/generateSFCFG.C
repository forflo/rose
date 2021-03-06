#include "rose.h"
#include <virtualCFG.h>
#include <cfgToDot.h>
#include <list>
#include "filteredCFG.h"
#include "DominatorTree.h"
#include "DominanceFrontier.h"
#include "DependenceGraph.h"
#include <string>

using namespace std;
using namespace VirtualCFG;
using namespace DominatorTreesAndDominanceFrontiers;

// SFCFG: Slice Filtered Control Flow Graph

int main(int argc, char **argv)
{

    SgProject *project = frontend(argc, argv);
    string outputFileName=project->get_fileList().front()->get_sourceFileNameWithoutPath ();

 // list < SgNode * >fd = NodeQuery::querySubTree(project, V_SgFunctionDefinition);
    NodeQuerySynthesizedAttributeType fd = NodeQuery::querySubTree(project, V_SgFunctionDefinition);
    // in the test example there is only one function, therefore we don't need 
    // to iterate over all functions
    // fd functionDefinition
 // for (list < SgNode * >::iterator i = fd.begin(); i != fd.end(); i++)
    for (NodeQuerySynthesizedAttributeType::iterator i = fd.begin(); i != fd.end(); i++)
    {
        SgFunctionDefinition *fDef = isSgFunctionDefinition(*i);

				SliceCFGNode cfg=SliceCFGNode(fDef->cfgForEnd());

				string filename;
				
				filename =outputFileName+"."+(fDef->get_declaration()->get_name().getString())+".CFG.dot";
				
        std::ofstream of(filename.c_str());
        cfgToDot(of, string("cfg"), cfg);
        of.close();
				/*
        TemplatedDominatorTree < IsStatementFilter > dt(*i,
                                                        (argc >
                                                         2) ? POST_DOMINATOR : PRE_DOMINATOR);
        TemplatedDominanceFrontier < IsStatementFilter > df(dt);				
        dt.writeDot("dt.dot");
				df.printFrontiers();*/
    }
    return 0;
}
