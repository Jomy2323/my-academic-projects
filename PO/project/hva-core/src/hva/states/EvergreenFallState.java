package hva.states;

import java.io.Serializable;

import hva.Tree;

public class EvergreenFallState extends SeasonState implements
 Serializable {

    public EvergreenFallState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 1;
    }

    @Override
    public String biologicalCycle() {
        return "COMFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new EvergreenWinterState(_tree));
        _tree.incrementTreeAge();
    }
}
