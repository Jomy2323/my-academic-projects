package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.TreeMap;
import hva.Tree;
import hva.Animal;
import hva.CaseInsensitiveComparator;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.stream.Collectors;


public class Habitat implements Serializable{

    /** habitat's id */
    private String _habitatId;

    /** habitat's name */
    private String _habitatName;

    /** habitat's area */
    private int _habitatArea;

    /** habitat's population */
    private int _habitatPopulation = 0;

    /** habitat's number of handlers */
    private int _habitatNumberOfHandlers = 0;

    /** habitat's species adequacy */
    private Map<String,Integer> _speciesAdequacy;

    /** habitat's trees */
    private Map<String, Tree> _habitatTrees;

    /** habitat's animals */
    private Map<String,Animal> _habitatAnimals;

    // Constructor method
    public Habitat(String id, String name, int area) {
        _habitatId = id;
        _habitatName = name;
        _habitatArea = area;
        _speciesAdequacy = new HashMap<String, Integer>();
        _habitatTrees = new TreeMap<String, Tree>
         (new CaseInsensitiveComparator());
        _habitatAnimals = new TreeMap<String, Animal>(new 
        CaseInsensitiveComparator());
    }

    // Getter methods
    public String getHabitatId() {
        return _habitatId;
    }

    public String getHabitatName() {
        return _habitatName;
    }

    public int getHabitatArea() {
        return _habitatArea;
    }

    public int getHabitatPopulation() {
        return _habitatPopulation;
    }

    public int getHabitatNumberOfHandlers() {
        return _habitatNumberOfHandlers;
    }

    public Map<String, Integer> getSpeciesAdequacy() {
        return _speciesAdequacy;
    }

    public Map<String, Tree> getHabitatTrees() {
        return _habitatTrees;
    }

    public Map<String, Animal> getHabitatAnimals() {
        return _habitatAnimals;
    }

    // Setter methods
    public void setHabitatArea(int area){
        _habitatArea = area;
    }

    public void incrementHabitatPopulation(){
        _habitatPopulation += 1;
    }

    public void decreaseHabitatPopulation(){
        _habitatPopulation -=1;
    }

    public void incrementHabitatNumberOfHandlers(){
        _habitatNumberOfHandlers += 1;
    }

    public void decreaseHabitatNumberOfHandlers(){
        _habitatNumberOfHandlers -= 1;
    }

    public void addTree(Tree tree){
        _habitatTrees.put(tree.getTreeId(), tree);
    }

    public void setHabitatInfluence(String speciesId,
     String influence){
        int influenceValue = 0;
        if (influence.equals("POS")){
            influenceValue = 20;
        } else if (influence.equals("NEG")){
            influenceValue = -20;
        } else if (influence.equals("NEU")){
            influenceValue = 0;
        }
        _speciesAdequacy.put(speciesId, influenceValue);
    }

    /**
     * Add an animal to the habitat
     * 
     * @param animal the animal to be added
     */
    public void addAnimal(Animal animal){
        _habitatAnimals.put(animal.getAnimalId(), animal);
    }

    /**
     * Remove an animal from the habitat
     * 
     * @param animalId the animal's id
     */
    public void removeAnimal(String animalId){
        _habitatAnimals.remove(animalId);
    }

    @Override
    public String toString() {
        return "HABITAT|" + _habitatId + "|" + _habitatName + "|" +
         _habitatArea + "|" + _habitatTrees.size();
    }
}