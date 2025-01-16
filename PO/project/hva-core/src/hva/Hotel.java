package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.Collection;
import java.util.Collections;
import java.util.stream.Collectors;
import java.util.Comparator;
import hva.exceptions.*;
import hva.states.*;

public class Hotel implements Serializable {

    @Serial
    private static final long serialVersionUID = 202407081733L;

    /** Species */
    private Map<String, Species> _hotelSpecies;
    /** Habitats */
    private Map<String, Habitat> _hotelHabitats;
    /** Animals */
    private Map<String, Animal> _hotelAnimals;
    /** Trees */
    private Map<String, Tree> _hotelTrees;
    /** Employees */
    private Map<String, Employee> _hotelEmployees;
    /** Vaccines */
    private Map<String, Vaccine> _hotelVaccines;

    /** Vaccination history */
    private List<VaccinationEvent> _hotelVaccinationHistory;

    /** Hotel object has been changed. */
    private boolean _changed = false;

    /** Hotel current season */
    private Season _hotelCurrentSeason = Season.SPRING;

    /**
     * Constructor method
     */
    public Hotel(){
        _hotelSpecies = new TreeMap<String, Species>(new
         CaseInsensitiveComparator());
        _hotelHabitats = new TreeMap<String, Habitat>(new 
        CaseInsensitiveComparator());
        _hotelAnimals = new TreeMap<String, Animal>(new 
        CaseInsensitiveComparator());
        _hotelTrees = new HashMap<String, Tree>();
        _hotelEmployees = new TreeMap<String, Employee>(new 
        CaseInsensitiveComparator());
        _hotelVaccines = new TreeMap<String, Vaccine>(new 
        CaseInsensitiveComparator());
        _hotelVaccinationHistory = new ArrayList<VaccinationEvent>();
    }

    /**
     * @return changed
    */
    public boolean hasChanged() {
        return _changed;
    }

    /**
     * @param changed
    */
    public void setChanged(boolean changed) {
        _changed = changed;
    }

    // getter methods

    public List<VaccinationEvent> getHotelVaccinationHistory(){
        return _hotelVaccinationHistory;
    }

    
    /**
     * Read text input file and create domain entities.
     *
     * @param filename name of the text input file
     * @throws ImportFileException
     */
    void importFile(String filename) throws ImportFileException {
        try (BufferedReader reader = new BufferedReader
        (new FileReader(filename))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] fields = line.split("\\|");
                try {
                    registerEntry(fields);
                } catch (UnrecognizedEntryException|
                    DuplicateAnimalException| UnknownHabitatException| 
                    DuplicateVaccineException| UnknownSpeciesException
                    |DuplicateTreeException| UnknownAnimalException|
                     DuplicateHabitatException|
                     DuplicateEmployeeException| 
                     DuplicateSpeciesException e) {
                    e.printStackTrace();
                }
            }
        } catch (IOException e1) {
            throw new ImportFileException(filename, e1);
        }

    }

    /**
     * Register a new entity in the hotel.
     *
     * @param fields entity fields
     * @throws UnrecognizedEntryException
     * @throws DuplicateAnimalException
     * @throws UnknownHabitatException
     * @throws DuplicateVaccineException
     * @throws UnknownSpeciesException
     * @throws DuplicateTreeException
     * @throws UnknownAnimalException
     * @throws DuplicateHabitatException
     * @throws DuplicateEmployeeException
     */
    public void registerEntry(String... fields) throws 
            UnrecognizedEntryException, 
            DuplicateAnimalException, UnknownHabitatException, 
            DuplicateVaccineException, UnknownSpeciesException, 
            DuplicateTreeException, DuplicateTreeException, 
            UnknownAnimalException, DuplicateHabitatException,
            DuplicateEmployeeException, DuplicateSpeciesException {
        switch (fields[0]) {
            case "ESPÉCIE" -> registerSpecies(fields);
            case "ANIMAL" -> registerAnimal(fields);
            case "HABITAT" -> registerHabitat(fields);
            case "ÁRVORE" -> registerTree(fields);
            case "VACINA" -> registerVaccine(fields);
            case "VETERINÁRIO", "TRATADOR" -> 
            registerEmployee(fields);
            default -> {throw new 
                UnrecognizedEntryException(fields[0]);}
        }
    }

    //Species methods

    /**
     * This method registers a new Species to the hotel. It can be 
     * called in two different ways, either by the DoRegisterAnimal
     * command from the app when the Species don't exist, or when 
     * importing a file with entities to be created. This method 
     * takes a vector of Strings as an argument. When the method is 
     * called by the app command it passes a null String at fields[0] 
     * because that is only needed on the import file method.
     * 
     * @param fields    Format: ESPÉCIE|id|nome
     * 
     * @throws DuplicateAnimalException thrown when there is already 
     *                                  an animal in the hotel with
     *                                  the given key
     * @throws UnknownHabitatException  thrown when the habitat key
     *                                  doesn´t exist in the hotel
     * @throws DuplicateSpeciesException thrown when there is already
     *                                   a species in the hotel with
     */
    public void registerSpecies(String... fields) throws
     DuplicateAnimalException, UnknownHabitatException, 
     DuplicateSpeciesException {
        if (_hotelSpecies.containsKey(fields[1]) ||
        _hotelSpecies.values().stream()
        .anyMatch(species -> species.getSpeciesName()
        .equalsIgnoreCase(fields[2]))) {
            throw new DuplicateSpeciesException();
        }


        Species species = new Species(fields[1], fields[2]);
        _hotelSpecies.put(fields[1], species);
        if (fields.length > 3){
            Habitat habitat = _hotelHabitats.get(fields[4]);
            Animal animal = new Animal(fields[3], fields[5],
             fields[1], habitat);
            habitat.incrementHabitatPopulation();
            habitat.addAnimal(animal);
            species.incrementSpeciesPopulation();
            species.addAnimal(fields[3]);
            _hotelAnimals.put(fields[3], animal);
        }
        setChanged(true);
    }

    /**
     * @param speciesId
     * 
     * @return if the species exists
     */
    public boolean speciesExists(String speciesId){
        return _hotelSpecies.containsKey(speciesId);
    }

    /**
     * @return all species in the hotel
     */
    public Map<String, Species> getHotelSpecies(){
        return _hotelSpecies;
    }

    //Animal methods

    /**
     * This method registers a new animal to the hotel. It can be 
     * called in two different ways, either by the DoRegisterAnimal 
     * command from the app, or when importing a file with entities
     * to be created. This method takes a vector of Strings as an 
     * argument. When the method is called by the app command it
     * passes a null String at fields[0] because that is only needed
     * on the import file method.
     * 
     * @param fields    Format: ANIMAL|id|name|id_species|id_habitat
     * 
     * @throws DuplicateAnimalException thrown when there is already 
     *                                  an animal in the hotel with 
     *                                  the given key
     * @throws UnknownHabitatException  thrown when the habitat key 
     *                                  doesn´t exist in the hotel
     */
    public void registerAnimal(String... fields) throws
     DuplicateAnimalException, UnknownHabitatException{
        if (_hotelAnimals.containsKey(fields[1])){throw new 
            DuplicateAnimalException(fields[1]);}
        if (!_hotelHabitats.containsKey(fields[4])){throw new
             UnknownHabitatException(fields[4]);}
        
        Habitat habitat = _hotelHabitats.get(fields[4]);
        Animal animal = new Animal(fields[1], fields[2], fields[3],
         habitat);
        Species species = _hotelSpecies.get(fields[3]);

        if (species != null){
            habitat.incrementHabitatPopulation();
            habitat.addAnimal(animal);
            species.incrementSpeciesPopulation();
            species.addAnimal(fields[1]);
            _hotelAnimals.put(fields[1], animal);
        }

        setChanged(true);
    }

    /**
     * @param animalId
     * 
     * @return an animal in the hotel
     * 
     * @throws UnknownAnimalException thrown when the animal key
     *                                doesn´t exist in the hotel
     */
    public Animal getHotelAnimal(String animalId) throws
     UnknownAnimalException {
        Animal animal = _hotelAnimals.get(animalId);
        if (animal == null) {
            throw new UnknownAnimalException(animalId);
        }
        return animal;
    }

    /**
     * Show all animals in the hotel.
     */
    public List<String> showAllAnimals() {
        return _hotelAnimals.values()
                             .stream()
                             .map(Animal::toString)
                             .collect(Collectors.toList());
    }

    /**
     * This method transfers an animal to a new habitat.
     * 
     * @param animalId
     * @param habitatId
     * 
     * @throws UnknownHabitatException  thrown when the habitat key
     *                                  doesn´t exist in the hotel
     */
    public void transferAnimalToHabitat(String animalId,
     String habitatId) throws UnknownHabitatException,
      UnknownAnimalException{
        if (!_hotelAnimals.containsKey(animalId)){
            throw new UnknownAnimalException(animalId);
        }
        if (!_hotelHabitats.containsKey(habitatId)){
            throw new UnknownHabitatException(habitatId);
        }

        Animal animal = _hotelAnimals.get(animalId);
        Habitat oldHabitat = animal.getAnimalHabitat();
        oldHabitat.decreaseHabitatPopulation();
        oldHabitat.removeAnimal(animalId);
        Habitat newHabitat = _hotelHabitats.get(habitatId);
        animal.setAnimalHabitat(newHabitat);
        newHabitat.incrementHabitatPopulation();
        newHabitat.addAnimal(animal);
    }
        
    /**
     * @param animalId
     * 
     * @return if the animal exists
     */
    public boolean animalExists(String animalId){
        return _hotelAnimals.containsKey(animalId);
    }

    /**
     * This function asks for an animal key. If the animal isn't
     * registered in the hotel, then the method throws an exception. 
     * Else, it calculates the satisfaction of the animal calling 
     * its calculateSatisfaction method.
     * 
     * @param animalKey
     * @return an int representing the satisfaction
     * @throws UnknownAnimalException thrown when animal isn´t
     *                                registered in the hotel
     */
    public int calculateAnimalSatisfaction(String animalKey)
     throws UnknownAnimalException{
        if (!_hotelAnimals.containsKey(animalKey)){throw new
            UnknownAnimalException(animalKey);}
       Animal animal = _hotelAnimals.get(animalKey);
       return animal.calculateSatisfaction();
    }



    //Habitat methods

    /**
     * This method registers a new habitat to the hotel. It can be
     *  called in two different ways, either by the DoRegisterHabitat
     *  command from the app, or when importing a file with entities
     *  to be created. This method takes a vector of Strings as an
     *  argument. When the method is called by the app command it
     *  passes a null String at fields[0] because that is only needed
     *  on the import file method.
     * 
     * @param fields    Format: HABITAT|id|name|area|trees
     * 
     * @throws DuplicateHabitatException thrown when there is already
     *                                   an habitat in the hotel with
     *                                   the given key
     */
    public void registerHabitat(String... fields) throws
     DuplicateHabitatException{
        if (_hotelHabitats.containsKey(fields[1])){throw
             new DuplicateHabitatException(fields[1]);}
        Habitat habitat = new Habitat(fields[1], fields[2],
         Integer.parseInt(fields[3]));
        if (fields.length > 4) {
            String[] trees = fields[4].split(",");
            for (String treeId : trees) {
                Tree tree = _hotelTrees.get(treeId);
                habitat.addTree(tree);
            }
        }
        _hotelHabitats.put(fields[1], habitat);
        setChanged(true);
    }

    /**
     * This method changes the area of a habitat.
     * 
     * @param habitatId
     * @param newArea
     */
    public void changeHabitatArea(String habitatId, int newArea) 
     throws UnknownHabitatException{
        if (!_hotelHabitats.containsKey(habitatId)){
            throw new UnknownHabitatException(habitatId);
        }
        Habitat habitat = _hotelHabitats.get(habitatId);
        habitat.setHabitatArea(newArea);
    }

    /**
     * This method changes the influence of a species in a habitat.
     * 
     * @param habitatId
     * @param speciesId
     * @param influence
     */
    public void changeHabitatInfluence(String habitatId,String
     speciesId, String influence) throws UnknownHabitatException,
      UnknownSpeciesException{
        if (!_hotelHabitats.containsKey(habitatId)) {
            throw new UnknownHabitatException(habitatId);
        }
        if (!_hotelSpecies.containsKey(speciesId)) {
            throw new UnknownSpeciesException(speciesId);
        }
        Habitat habitat = _hotelHabitats.get(habitatId);
        habitat.setHabitatInfluence(speciesId, influence);
    }

    /**
     * Show all habitats in the hotel.
     */
    public List<String> showAllHabitats() {
        return _hotelHabitats.values().stream()
                             .flatMap(habitat -> {
                                 List<String> lines = new
                                  ArrayList<>();
                                 lines.add(habitat.toString()); 
    
                                 try{
                                    lines.addAll
                                    (showTreesInHabitat(habitat.
                                    getHabitatId()));
                                 }
                                 catch (UnknownHabitatException e){
                                    e.printStackTrace();
                                 }
                                 return lines.stream(); 
                             })
                             .collect(Collectors.toList());
    }

    /**
     * Show all trees in the habitat.
     */
    public List<String> showTreesInHabitat(String habitatKey) throws
     UnknownHabitatException {
        Habitat habitat = getHotelHabitat(habitatKey);
        return habitat.getHabitatTrees().values()
                      .stream()
                      .map(Tree::toString) 
                      .collect(Collectors.toList());
    }    

    /**
     * @param habitatId
     * 
     * @return if the habitat exists
     */
    public boolean habitatExists(String habitatId){
        return _hotelHabitats.containsKey(habitatId);
    }

    /**
     * @param habitatId
     * 
     * @return the habitat itself
     * 
     * @throws UnknownHabitatException thrown when the habitat key
     *                                 doesn´t exist in the hotel
     */
    public Habitat getHotelHabitat(String habitatId) throws
     UnknownHabitatException {
        Habitat habitat = _hotelHabitats.get(habitatId); 
        if (habitat == null) {
            throw new UnknownHabitatException(habitatId);
        }
        return habitat;
    }

    //Tree methods

    /**
     * This method registers a new tree to the hotel. It can be 
     * called in two different ways, either by the DoAddTreetoHabitat 
     * command from the app, or when importing a file with entities
     * to be created. This method takes a vector of Strings as an
     * argument. When the method is called by the app command it
     * passes a null String at fields[0] because that is only needed
     * on the import file method.
     * 
     * @param fields    Format: ÁRVORE|id|nome|idade|dificuldade|tipo
     * 
     * @throws DuplicateTreeException thrown when there is already an
     *                                tree in the hotel with the given
     *                                key
     * 
     * @throws UnknownHabitatException thrown when the habitat key
     *                                 doesn´t exist in the hotel
     * 
     * @return the tree
     */
    public Tree registerTree(String... fields) throws
     DuplicateTreeException, UnknownHabitatException{

        String treeId = fields[1];
        String treeName = fields[2];
        int age = Integer.parseInt(fields[3]);;
        int cleaningDifficulty = Integer.parseInt(fields[4]);;
        String treeType = fields[5];


        if (_hotelTrees.containsKey(treeId)){throw new
             DuplicateTreeException(treeId);}
        // Only checks if its invoked through DoAddTreeToHabitat
        if (fields.length > 6) {
            String habitatId = fields[6];
            if (!_hotelHabitats.containsKey(habitatId)) {
                throw new UnknownHabitatException(habitatId);
            }
        }
        Tree tree; 
        switch (treeType) {
            case "CADUCA" -> {
                tree = new DeciduousTree(treeId, treeName, age,
                 cleaningDifficulty);
                tree.setState(getInitialDeciduousSeasonState(tree));
            }
            case "PERENE" -> {
                tree = new EvergreenTree(treeId, treeName, age,
                 cleaningDifficulty);
                tree.setState(getInitialEvergreenSeasonState(tree));
            }
            default -> throw new
             IllegalArgumentException("Unknown tree type: " +
              treeType);
        }
        
        _hotelTrees.put(treeId, tree);
        setChanged(true);
        return tree;
    }

    /**
     * This method adds a tree to a habitat.
     * 
     * @param habitatId
     * @param tree
     */
    public Tree addTreeToHabitat(String habitatId, String treeKey){
        Habitat habitat = _hotelHabitats.get(habitatId);
        Tree tree = _hotelTrees.get(treeKey);
        habitat.addTree(tree);
        return tree;
    }

    /**
     * @param treeId
     * 
     * @return if the tree exists
     */
    public boolean treeExists(String treeId){
        return _hotelTrees.containsKey(treeId);
    }

    //Vaccine methods

    /**
     * This method registers a new vaccine to the hotel. It can be
     * called in two different ways, either by the DoRegisterVaccine
     * command from the app, or when importing a file with entities
     * to be created. This method takes a vector of Strings as an
     * argument. When the method is called by the app command it
     * passes a null String at fields[0] because that is only needed
     * on the import file method.
     * 
     * @param fields    Format: VACINA|id|nome|id_species
     * 
     * @throws DuplicateVaccineException thrown when there is already
     *                                   an vaccine in the hotel with
     *                                   the given key
     * 
     * @throws UnknownSpeciesException thrown when the species key
     *                                 doesn´t exist in the hotel
     */
    public void registerVaccine(String... fields) throws
     DuplicateVaccineException, UnknownSpeciesException{
        if (_hotelVaccines.containsKey(fields[1])){throw new
             DuplicateVaccineException(fields[1]);}
        
        Vaccine vaccine = new Vaccine(fields[1], fields[2]);
        if (fields.length > 3){
            for (String speciesId : fields[3].split(",")) {
                if (!_hotelSpecies.containsKey(speciesId)){throw new
                     UnknownSpeciesException(speciesId);}
                vaccine.addVaccinableSpecies(speciesId);
            }
        }
        _hotelVaccines.put(fields[1], vaccine);
        setChanged(true);
    }

    /**
     * Show all vaccines in the hotel.
     */
    public List<String> showAllVaccines() {
        return _hotelVaccines.values()
                             .stream()
                             .map(Vaccine::toString)
                             .collect(Collectors.toList());
    }

    /**
     * Show all vaccines in the hotel.
     */
    public List<String> showAllVaccinations() {
        return _hotelVaccinationHistory.stream()
                        .map(VaccinationEvent::toString)
                        .collect(Collectors.toList());
    }

    /**
     * @param vaccineId
     * @param vetId
     * @param animalId
     * 
     * @throws UnknownVaccineException thrown when the vaccine key
     *                                 doesn´t exist in the hotel
     * 
     * @throws UnknownAnimalException thrown when the animal key
     *                                doesn´t exist in the hotel
     * 
     * @throws UnknownVeterinarianException thrown when the vet key
     *                                     doesn´t exist in the hotel
     * 
     * @throws VeterinarianNotAuthorizedException thrown when the vet
     *                                           is not authorized to
     *                                           vaccinate the animal
     * 
     * @return if the vaccine is wrong
     */
    public boolean vaccinateAnimal(String vaccineId, String vetId,
     String animalId) throws UnknownVaccineException,
     UnknownAnimalException, UnknownVeterinarianException,
     VeterinarianNotAuthorizedException{
        
        if (!_hotelVaccines.containsKey(vaccineId)){throw new
             UnknownVaccineException(vaccineId);}
        if (!vetExists(vetId)){throw new
        UnknownVeterinarianException(vetId);}
        if (!_hotelAnimals.containsKey(animalId)){throw new
             UnknownAnimalException(animalId);}
        Vet vet = (Vet) _hotelEmployees.get(vetId);
        Animal animal = _hotelAnimals.get(animalId);
        Vaccine vaccine = _hotelVaccines.get(vaccineId);
        if (!vet.getResponsabilityKey().containsKey(
         animal.getAnimalSpecies())){
            throw new VeterinarianNotAuthorizedException(vetId,
             animal.getAnimalSpecies());
        }
        addVaccinationEvent(vaccine, animal, vet);
        if (!vaccine.getVaccinableSpecies().contains(
         animal.getAnimalSpecies())){
            return true;
        }
        return false;
    }

    /**
     * This method adds a vaccination event.
     * 
     * @param vaccine
     * @param animal
     * @param vet
     */
    public void addVaccinationEvent(Vaccine vaccine, Animal animal,
     Vet vet){
        String effect = calculateEffect(vaccine, animal);
        VaccinationEvent vaccinationEvent = new VaccinationEvent(
         vaccine, vet, _hotelSpecies.get(animal.getAnimalSpecies()),
         effect);

        _hotelVaccinationHistory.add(vaccinationEvent);
        vaccine.incrementNumberOfApplications();

        animal.addVaccinationHistory(vaccinationEvent);
        vet.addVaccinationHistory(vaccinationEvent);
    }

    /**
     * This method calculates the effect of a vaccine on an animal.
     * 
     * @param vaccine
     * @param animal
     * 
     * @return the effect
     */
    public String calculateEffect(Vaccine vaccine, Animal animal) {
        String speciesName = _hotelSpecies.get(
         animal.getAnimalSpecies()).getSpeciesName();
        Set<String> vaccinableSpecies=vaccine.getVaccinableSpecies();

        if (vaccinableSpecies.contains(animal.getAnimalSpecies())) {
            return "NORMAL";
        }

        int maxDamage = 0;
        for (String species : vaccinableSpecies) {
            species = _hotelSpecies.get(species).getSpeciesName();
            int maxLength = Math.max(speciesName.length(), 
             species.length());
            int commonCharacters = countCommonCharacters(speciesName,
             species);
            int damage = maxLength - commonCharacters;
            maxDamage = Math.max(maxDamage, damage);
        }

        if (maxDamage == 0) {
            return "CONFUSÃO";
        } else if (maxDamage >= 1 && maxDamage <= 4) {
            return "ACIDENTE";
        } else {
            return "ERRO";
        }
    }

    /**
     * This method counts the common characters between two strings.
     * 
     * @param species1
     * @param species2
     * 
     * @return the number of common characters
     */
    public int countCommonCharacters(String species1, String species2)
     {
        species1 = species1.toLowerCase();
        species2 = species2.toLowerCase();
    
        Set<Character> species1Chars = new HashSet<>();
        for (char c : species1.toCharArray()) {
            species1Chars.add(c);
        }
    
        Set<Character> species2Chars = new HashSet<>();
        for (char c : species2.toCharArray()) {
            species2Chars.add(c);
        }
    
        species1Chars.retainAll(species2Chars);
    
        return species1Chars.size();
    }
    

    

    //Employee methods

    /**
     * This method selects the type of employee that is going to
     * register. It can be called in two different ways, either by the
     * DoRegisterEmployee command from the app, or when importing a
     * file with entities to be created. This method takes a vector of
     * Strings as an argument.
     * 
     * @param fields    Format: type|id|name|responsabilities
     * 
     * @throws DuplicateEmployeeException thrown when there is already
     *                                    an employee in the hotel
     *                                    with the given key
     */
    public void registerEmployee(String... fields) throws
     DuplicateEmployeeException{
        if (_hotelEmployees.containsKey(fields[1])){throw new
             DuplicateEmployeeException(fields[1]);}

        Employee employee;
        switch (fields[0]) {
            case "VETERINÁRIO" -> registerVet(fields);
            case "TRATADOR" -> registerHandler(fields);
            default -> employee = null;
        }
        setChanged(true);
    }

    /**
     * This method registers a new vet to the hotel.
     */
    public void registerVet(String... fields){
        Vet vet = new Vet(fields[1], fields[2]);
        if (fields.length > 3) {
            String[] speciesList = fields[3].split(",");
            for (String speciesId : speciesList) {
                Species species = _hotelSpecies.get(speciesId);
                vet.addVaccinableSpecies(speciesId, species);
                species.incrementSpeciesNumberOfVets();
            }
        }
        _hotelEmployees.put(fields[1], vet);
    }

    /**
     * This method registers a new handler to the hotel.
     */
    public void registerHandler(String... fields){
        Handler handler = new Handler(fields[1], fields[2]);
        if (fields.length > 3) {
            String[] habitats = fields[3].split(",");
            for (String habitatId : habitats) {
                Habitat habitat = _hotelHabitats.get(habitatId);
                handler.addHandlerHabitat(habitatId, habitat);
                habitat.incrementHabitatNumberOfHandlers();
            }
        }
        _hotelEmployees.put(fields[1], handler);
    }

    /**
     * @param employeeId
     * 
     * @return a vet in the hotel
     * 
     * @throws UnknownVeterinarianException thrown when the vet key
     *                                     doesn´t exist in the hotel
     */
    public Vet getHotelVet(String vetId) 
     throws UnknownVeterinarianException {
        Vet vet = (Vet) _hotelEmployees.get(vetId);
        if (vet == null) {
            throw new UnknownVeterinarianException(vetId);
        }
        return vet;
    }
    /**
     * Show all employees in the hotel.
     */
    public List<String> showAllEmployees() {
        return _hotelEmployees.values()
                             .stream()
                             .map(Employee::toString)
                             .collect(Collectors.toList());
    }

    /**
     * This method asks for an employee id and a responsability id
     * and having in consideration the type of employee, proceeds to
     * check if the responsability given is adequate to the given 
     * employee type. If not, it throws the adequate exception. Else,
     * it adds the responsability to the map.
     * @param employeeId
     * @param responsability
     * 
     * @throws UnknownEmployeeException thrown when the employee key
     *                                  doesn´t exist in the hotel
     * 
     * @throws NoResponsabilityException thrown when the
     *                                   responsability key doesn´t
     *                                   exist in the hotel
     */
    public void addResponsability(String employeeKey,
     String responsability) throws UnknownEmployeeException,
     NoResponsabilityException{
        if (!_hotelEmployees.containsKey(employeeKey)){
            throw new UnknownEmployeeException(employeeKey);
        }
        Employee employee = _hotelEmployees.get(employeeKey);

        switch (employee.getClass().getSimpleName()) {
            case "Vet" -> addVetResponsability(employee,
             responsability);
            case "Handler" -> addHandlerResponsability(employee,
             responsability);
            default -> throw new
             IllegalArgumentException("No such employee:" + employee);
        }
    }

    /**
     * @param employee
     * @param responsability
     * 
     * @throws NoResponsabilityException thrown when the
     *                                   responsability key doesn´t
     *                                   exist in the hotel
     */
    public void addVetResponsability(Employee employee,
     String responsability) throws NoResponsabilityException{
        if (!_hotelSpecies.containsKey(responsability)){throw new
             NoResponsabilityException(employee.getEmployeeId(),
             responsability);}
        Vet vet = (Vet) employee;

        if (!vet.getResponsabilityKey().containsKey(responsability)){
            Species species = _hotelSpecies.get(responsability);
            vet.addVaccinableSpecies(responsability, species);
            species.incrementSpeciesNumberOfVets();
        }
    }

    /**
     * @param employee
     * @param responsability
     * 
     * @throws NoResponsabilityException thrown when the
     *                                   responsability key doesn´t
     *                                   exist in the hotel
     */
    public void addHandlerResponsability(Employee employee,
     String responsability) throws NoResponsabilityException{
        if (!_hotelHabitats.containsKey(responsability)){throw new
             NoResponsabilityException(employee.getEmployeeId(),
              responsability);}

        Handler handler = (Handler) employee;
        if (!handler.getResponsabilityKey().containsKey
        (responsability)){
            Habitat habitat = _hotelHabitats.get(responsability);
            handler.addHandlerHabitat(responsability, habitat);
            habitat.incrementHabitatNumberOfHandlers();
        }
    }

    /**
     * @param employeeId
     * @param responsability
     * 
     * @throws UnknownEmployeeException thrown when the employee key
     *                                  doesn´t exist in the hotel
     * 
     * @throws NoResponsabilityException thrown when the responsability
     *                                   key doesn´t exist in the hotel
     */
    public void removeResponsability(String employeeKey,
     String responsability) throws UnknownEmployeeException,
     NoResponsabilityException{
        if (!_hotelEmployees.containsKey(employeeKey)){throw new
             UnknownEmployeeException(employeeKey);}
        Employee employee = _hotelEmployees.get(employeeKey);

        switch (employee.getClass().getSimpleName()) {
            case "Vet" -> removeVetResponsability(employee,
             responsability);
            case "Handler" -> removeHandlerResponsability(employee,
             responsability);
            default -> throw new
             IllegalArgumentException("No such employee:" + employee);
        }
    }

    /**
     * @param employee
     * @param responsability
     * 
     * @throws NoResponsabilityException thrown when the 
     *                                   responsability key doesn´t
     *                                   exist in employee
     *                                   responsabilities
     */
    public void removeVetResponsability(Employee employee,
     String responsability) throws NoResponsabilityException{
        Vet vet = (Vet) employee;
        if (!vet.getResponsabilityKey().containsKey(
         responsability)){
            throw new NoResponsabilityException
             (employee.getEmployeeId(),responsability);
        }
        Species species = vet.getResponsabilityKey().get(
         responsability);
        vet.removeVaccinableSpecies(responsability);
        species.decrementSpeciesNumberOfVets();
    }

    /**
     * @param employee
     * @param responsability
     * 
     * @throws NoResponsabilityException thrown when the 
     *                                   responsability key doesn´t
     *                                   exist in employee
     *                                   responsabilities
     */
    public void removeHandlerResponsability(Employee employee,
     String responsability) throws NoResponsabilityException{
        Handler handler = (Handler) employee;
        if (!handler.getResponsabilityKey().containsKey
        (responsability)){
            throw new NoResponsabilityException
             (employee.getEmployeeId(), responsability);
        }
        Habitat habitat = handler.getResponsabilityKey().get
        (responsability);
        handler.removeHandlerHabitat(responsability);
        habitat.decreaseHabitatNumberOfHandlers();
    }

    /**
     * @param employeeId
     * 
     * @return if the employee exists
     */
    public boolean vetExists(String vetId){
        return _hotelEmployees.containsKey(vetId) &&
         _hotelEmployees.get(vetId) instanceof Vet;
    }

    /**
     * This function asks for an employee key. If the employee isn't
     * registered in the hotel, then the method throws an exception. 
     * Else, it calculates the satisfaction of the employee calling 
     * its calculateSatisfaction method.
     * 
     * @param employeeKey
     * @return an int representing the satisfaction
     * @throws UnknownEmployeeException thrown when employee isn´t
     *                                  registered in the hotel
     */
    public int calculateEmployeeSatisfaction(String employeeKey)
     throws UnknownEmployeeException{
        if (!_hotelEmployees.containsKey(employeeKey)){throw new
            UnknownEmployeeException(employeeKey);}
       Employee employee = _hotelEmployees.get(employeeKey);
       return employee.getSatisfaction();
    }

    //Season methods

    /**
     * This method advances the Season according to the natural order
     * of seasons Spring < Summer < Fall < Winter. It updates the
     * season for the hotel itself and all its trees.
     * 
     * @return the season code number of the new season
     */
    public int advanceSeason() {
        _hotelCurrentSeason = _hotelCurrentSeason.next();
        for (Tree tree : _hotelTrees.values()){
            tree.nextSeason();
        }
        return _hotelCurrentSeason.ordinal();
    }

    public SeasonState getInitialDeciduousSeasonState(Tree tree) {
        switch (_hotelCurrentSeason) {
            case SPRING:
                return new DeciduousSpringState(tree);
            case SUMMER:
                return new DeciduousSummerState(tree);
            case FALL:
                return new DeciduousFallState(tree);
            case WINTER:
                return new DeciduousWinterState(tree);
            default:
                throw new IllegalArgumentException("Invalid season");
        }
    }

    public SeasonState getInitialEvergreenSeasonState(Tree tree) {
        switch (_hotelCurrentSeason) {
            case SPRING:
                return new EvergreenSpringState(tree);
            case SUMMER:
                return new EvergreenSummerState(tree);
            case FALL:
                return new EvergreenFallState(tree);
            case WINTER:
                return new EvergreenWinterState(tree);
            default:
                throw new IllegalArgumentException("Invalid season");
        }
    }

    // satisfaction methods
    public int calculateGlobalSatisfaction(){
        int globalSatisfaction = 0;

        for (Animal animal : _hotelAnimals.values()){
            globalSatisfaction += animal.
             calculateSatisfaction();
        }

        for (Employee employee : _hotelEmployees.values()){
            globalSatisfaction += employee.getSatisfaction();
        }

        return globalSatisfaction;
    }

}
