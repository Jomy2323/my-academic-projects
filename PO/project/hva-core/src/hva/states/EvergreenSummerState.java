package hva.states;

import java.io.Serializable;

import hva.Tree;

public class EvergreenSummerState extends SeasonState implements
 Serializable {

    public EvergreenSummerState(Tree tree) {
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
        _tree.setState(new EvergreenFallState(_tree));
        _tree.incrementTreeAge();
    }
}
