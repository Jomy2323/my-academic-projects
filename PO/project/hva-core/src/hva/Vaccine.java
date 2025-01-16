package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Set;
import java.util.HashSet;

public class Vaccine implements Serializable{

    /** vaccine's id */
    private String _vaccineId;

    /** vaccine's name */
    private String _vaccineName;

    /** number of applications */
    private int _numberOfApplications;

    /** vaccinable species */
    private Set<String> _vaccinableSpecies;

    /**
     * Constructor
     * @param vaccineId the vaccine's id
     * @param vaccineName the vaccine's name
     */
    public Vaccine(String vaccineId, String vaccineName) {
        this._vaccineId = vaccineId;
        this._vaccineName = vaccineName;
        this._numberOfApplications = 0;
        this._vaccinableSpecies = new HashSet<String>();
    }

    //getter methods
    public String getVaccineId() {
        return _vaccineId;
    }

    public String getVaccineName() {
        return _vaccineName;
    }

    public int getNumberOfApplications() {
        return _numberOfApplications;
    }

    public Set<String> getVaccinableSpecies() {
        return _vaccinableSpecies;
    }

    public void incrementNumberOfApplications() {
        _numberOfApplications++;
    }

    //ADD species to _vaccinableSpecies
    public void addVaccinableSpecies(String species) {
        this._vaccinableSpecies.add(species);
    }

    @Override
    public String toString() {
        StringBuilder result = new StringBuilder("VACINA|" + 
        _vaccineId + "|" + _vaccineName + "|" +
         _numberOfApplications);
        if (!_vaccinableSpecies.isEmpty()) {
            result.append("|" + String.join(",", _vaccinableSpecies));
        }
        return result.toString();
    }
}