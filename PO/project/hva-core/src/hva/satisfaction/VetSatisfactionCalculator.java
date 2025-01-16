package hva.satisfaction;

import java.io.Serializable;

import hva.Vet;
import hva.Species;

public class VetSatisfactionCalculator implements
 SatisfactionCalculator, Serializable{

    /** vet */
    private Vet _vet;

    /**
     * Constructor
     * 
     * @param vet the vet to calculate the satisfaction
     */
    public VetSatisfactionCalculator(Vet vet){
        _vet = vet;
    }

    /**
     * Calculate the satisfaction of a vet
     * 
     * @return the satisfaction of the vet
     */
    @Override
    public int calculateSatisfaction(){

        double work = 0;

        for (Species species : _vet.getResponsabilityKey().values()){
            double speciesPopulation = species.getSpeciesPopulation();
            double speciesNumberOfVets =
             species.getSpeciesNumberOfVets(); 
            work += speciesPopulation / speciesNumberOfVets;
        }

        return (int) Math.round(20 - work);
    }

}