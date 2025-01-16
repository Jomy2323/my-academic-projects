package hva.states;

import java.io.Serializable;

import hva.Tree;

public class DeciduousSummerState extends SeasonState implements
 Serializable {

    public DeciduousSummerState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 2;
    }

    @Override
    public String biologicalCycle() {
        return "COMFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new DeciduousFallState(_tree));
        _tree.incrementTreeAge();
    }
}