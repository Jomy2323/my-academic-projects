package hva.states;

import java.io.Serializable;

import hva.Tree;

public class DeciduousFallState extends SeasonState implements
 Serializable{

    public DeciduousFallState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 5;
    }

    @Override
    public String biologicalCycle() {
        return "LARGARFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new DeciduousWinterState(_tree));
        _tree.incrementTreeAge();
    }
}
