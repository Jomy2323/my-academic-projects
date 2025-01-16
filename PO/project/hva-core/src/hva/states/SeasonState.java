package hva.states;

import hva.Tree;

public abstract class SeasonState {
    protected Tree _tree;

    public SeasonState(Tree tree) {
        _tree = tree;
    }

    public abstract int seasonalEffort();
    public abstract String biologicalCycle();
    public abstract void next();
}
