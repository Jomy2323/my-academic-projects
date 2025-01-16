package hva.satisfaction;

import java.io.Serializable;

import hva.Handler;
import hva.Habitat;
import hva.Tree;

public class HandlerSatisfactionCalculator implements
 SatisfactionCalculator, Serializable{

    /** handler */
    private Handler _handler;

    /**
     * Constructor
     * 
     * @param handler the handler to calculate the satisfaction
     */
    public HandlerSatisfactionCalculator(Handler handler){
        _handler = handler;
    }

    /**
     * Calculate the satisfaction of a handler
     * 
     * @return the satisfaction of the handler
     */
    @Override
    public int calculateSatisfaction(){
        double work = 0;

        for (Habitat habitat :
         _handler.getResponsabilityKey().values()){
            double habitatWork = 0;
            double habitatArea = habitat.getHabitatArea();
            double habitatPopulation = habitat.getHabitatPopulation();
            double habitatNumberOfHandlers = habitat.
             getHabitatNumberOfHandlers();
            double habitatTreesEffort = 0;
            for (Tree tree : habitat.getHabitatTrees().values()){
                habitatTreesEffort +=
                tree.getTreeBaseCleaningDifficulty() *
                 tree.seasonalEffort() *
                  Math.log(tree.getAgeInYears() + 1);           
            }
            habitatWork += habitatArea + 3 * habitatPopulation +
             habitatTreesEffort;
            work += habitatWork / habitatNumberOfHandlers;
        }

        return (int) Math.round(300 - work);
    }

}