package hva.states;

import java.io.Serializable;

import hva.Tree;

public class DeciduousSpringState extends SeasonState implements
 Serializable {

    public DeciduousSpringState(Tree tree) {
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
        _tree.setState(new DeciduousSummerState(_tree));
        _tree.incrementTreeAge();
    }
}
