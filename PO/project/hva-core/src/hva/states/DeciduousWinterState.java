package hva.states;

import java.io.Serializable;

import hva.Tree;

public class DeciduousWinterState extends SeasonState implements
 Serializable {

    public DeciduousWinterState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 0;
    }

    @Override
    public String biologicalCycle() {
        return "SEMFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new DeciduousSpringState(_tree));
        _tree.incrementTreeAge();

    }
}
