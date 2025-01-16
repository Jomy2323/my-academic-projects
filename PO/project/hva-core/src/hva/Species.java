package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Set;
import java.util.HashSet;

public class Species implements Serializable{

    /** species' id */
    private String _speciesId;

    /** species' name */
    private String _speciesName;

    /** species' animals */
    private Set<String> _speciesAnimals;

    /** species' number of vets */
    private int _speciesNumberOfVets;

    /** species' population */
    private int _speciesPopulation;

    /**
     * Constructor
     * @param speciesId the species' id
     * @param speciesName the species' name
     */
    public Species(String speciesId, String speciesName) {
        _speciesId = speciesId;
        _speciesName = speciesName;
        _speciesAnimals = new HashSet<String>();
        _speciesNumberOfVets = 0;
        _speciesPopulation = 0;
    }

    // Getter methods
    public String getSpeciesId() {
        return _speciesId;
    }

    public String getSpeciesName() {
        return _speciesName;
    }

    public Set<String> getSpeciesAnimals() {
        return _speciesAnimals;
    }

    public int getSpeciesNumberOfVets() {
        return _speciesNumberOfVets;
    }

    public int getSpeciesPopulation() {
        return _speciesPopulation;
    }

    //ADD animal to _speciesAnimals
    public void addAnimal(String animalId) {
        _speciesAnimals.add(animalId);
    }

    // Setter methods
    public void setSpeciesNumberOfVets(int speciesNumberOfVets) {
        this._speciesNumberOfVets = speciesNumberOfVets;
    }

    public void incrementSpeciesPopulation(){
        _speciesPopulation += 1;
    }

    public void incrementSpeciesNumberOfVets(){
        _speciesNumberOfVets += 1;
    }

    public void decrementSpeciesNumberOfVets(){
        _speciesNumberOfVets -= 1;
    }
}