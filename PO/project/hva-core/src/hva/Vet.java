package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import hva.CaseInsensitiveComparator;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.stream.Collectors;

import hva.satisfaction.VetSatisfactionCalculator;
import hva.satisfaction.SatisfactionCalculator;

public class Vet extends Employee implements Serializable{

    /** vet's vaccinable species */
    private Map<String, Species> _vetVaccinableSpecies;

    /** vet's vaccination history */
    private List<VaccinationEvent> _vetVaccinationHistory;

    /**
     * Constructor
     * @param id the vet's id
     * @param name the vet's name
     */
    public Vet(String id, String name){
        super(id, name);
        _vetVaccinableSpecies = new TreeMap<String, Species>(
            new CaseInsensitiveComparator());
        _vetVaccinationHistory = new ArrayList<VaccinationEvent>();

        setSatisfactionCalculator(new
         VetSatisfactionCalculator(this));
    }

    // Getter methods
    public Map<String, Species> getResponsabilityKey() {
        return _vetVaccinableSpecies;
    }

    public List<VaccinationEvent> getVaccinationHistory() {
        return _vetVaccinationHistory;
    }

    public void addVaccinableSpecies(String speciesId,
     Species species){
        _vetVaccinableSpecies.put(speciesId, species);
    }

    public void removeVaccinableSpecies(String speciesId){
        _vetVaccinableSpecies.remove(speciesId);
    }

    public void addVaccinationHistory(VaccinationEvent event){
        _vetVaccinationHistory.add(event);
    }

    @Override
    public String toString() {
        if (_vetVaccinableSpecies.isEmpty()){
            return "VET|" + super.toString();
        } else {
            List<String> sortedKeys =
             new ArrayList<>(_vetVaccinableSpecies.keySet()); 
            String result = String.join(",", sortedKeys);
            return "VET|" + super.toString() + "|" + result;
        }
    }
}