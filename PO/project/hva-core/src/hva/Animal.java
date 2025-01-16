package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.List;
import hva.Species;
import hva.VaccinationEvent;
import java.util.List;
import java.util.ArrayList;
import hva.VaccinationEvent;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.stream.Collectors;

public class Animal implements Serializable{

    /** animal's id */
    private String _animalId;

    /** animal's name */
    private String _animalName;

    /** animal's species */
    private String _animalSpecies;

    /** animal's health history */
    private List<String> _animalHealthHistory;

    /** animal's vaccination history */
    private List<VaccinationEvent> _animalVaccinationHistory;

    /** animal's habitat */
    private Habitat _animalHabitat;

    /**
     * Constructor
     * @param animalId the animal's id
     * @param animalName the animal's name
     * @param animalSpecies the animal's species
     * @param animalHabitat the animal's habitat
     */
    public Animal(String animalId, String animalName,
     String animalSpecies, Habitat animalHabitat) {
        _animalId = animalId;
        _animalName = animalName;
        _animalSpecies = animalSpecies;
        _animalHealthHistory = new ArrayList<String>();
        _animalVaccinationHistory =
         new ArrayList<VaccinationEvent>();
        _animalHabitat = animalHabitat;
    }

    @Override
    public String toString() {
        if (_animalHealthHistory.isEmpty()){
            return "ANIMAL|" + _animalId + "|" + _animalName +
             "|" + _animalSpecies + "|VOID|" +
              _animalHabitat.getHabitatId();
        } else {
            String healthHistory =
             String.join(",", _animalHealthHistory);
            return "ANIMAL|" + _animalId + "|" + _animalName +
             "|" + _animalSpecies + "|" + healthHistory + "|" +
              _animalHabitat.getHabitatId();
        }
    }

    // Getter methods
    public String getAnimalId() {
        return _animalId;
    }

    public String getAnimalName() {
        return _animalName;
    }

    public String getAnimalSpecies() {
        return _animalSpecies;
    }

    public List<String> getAnimalHealthHistory() {
        return _animalHealthHistory;
    }

    public List<VaccinationEvent> getAnimalVaccinationHistory() {
        return _animalVaccinationHistory;
    }

    public Habitat getAnimalHabitat() {
        return _animalHabitat;
    }

    // Setter methods
    public void setAnimalHabitat(Habitat animalHabitat) {
        _animalHabitat = animalHabitat;
    }

    public void addVaccinationHistory(VaccinationEvent event){
        _animalVaccinationHistory.add(event);
        _animalHealthHistory.add(event.getEffect());
    }

    // satisfaction methods

    public int calculateSatisfaction() {
        double equals = countEquals();
        double different = _animalHabitat.getHabitatAnimals().
        size() - equals - 1;

        Integer adequacyValue = _animalHabitat.getSpeciesAdequacy().
        get(_animalSpecies);
        double adequacy = (adequacyValue != null) ? adequacyValue : 0;
        
        double area = _animalHabitat.getHabitatArea();
        double population = _animalHabitat.getHabitatPopulation();
    
        // Calculate satisfaction
        double satisfaction = 20 + 3 * equals - 2 * different + area
         / population + adequacy;
    
        return (int) Math.round(satisfaction);
    }

    public double countEquals(){
        int sameSpecies = 0;
        for (Animal animal : _animalHabitat.getHabitatAnimals()
         .values()) {
            if (_animalSpecies.equals(animal.getAnimalSpecies())){
                sameSpecies += 1;
            }
        }

        return sameSpecies - 1;
    }
}