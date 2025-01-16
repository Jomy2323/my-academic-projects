package hva.states;

import java.io.Serializable;

import hva.Tree;

public class EvergreenSpringState extends SeasonState implements
 Serializable {

    public EvergreenSpringState(Tree tree) {
        super(tree);
    }

    @Override
    public int seasonalEffort() {
        return 1;
    }

    @Override
    public String biologicalCycle() {
        return "GERARFOLHAS";
    }

    @Override
    public void next() {
        _tree.setState(new EvergreenSummerState(_tree));
        _tree.incrementTreeAge();
    }
}
