package hva.states;

import java.io.Serializable;

import hva.Tree;

public class EvergreenWinterState extends SeasonState implements
 Serializable {

    public EvergreenWinterState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 2;
    }

    @Override
    public String biologicalCycle() {
        return "LARGARFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new EvergreenSpringState(_tree));
        _tree.incrementTreeAge();
    }
}
