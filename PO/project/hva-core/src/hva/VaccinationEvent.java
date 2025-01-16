package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Set;
import hva.Vaccine;
import hva.Vet;
import hva.Species;

public class VaccinationEvent implements Serializable{

    /** vaccine */
    private Vaccine _vaccine;

    /** vet */
    private Vet _vet;

    /** species */
    private Species _species;

    /** effect */
    private String _effect;

    /**
     * Constructor
     * @param vaccine the vaccine
     * @param vet the vet
     * @param species the species
     * @param effect the effect
     */
    public VaccinationEvent(Vaccine vaccine, Vet vet,
     Species species, String effect) {
        _vaccine = vaccine;
        _vet = vet;
        _species = species;
        _effect = effect;
    }

    // Getter methods
    public Vaccine getVaccine() {
        return _vaccine;
    }

    public Vet getVet() {
        return _vet;
    }

    public Species getSpecies() {
        return _species;
    }

    public String getEffect() {
        return _effect;
    }

    @Override
    public String toString() {
        return "REGISTO-VACINA|" + _vaccine.getVaccineId() + "|" +
        _vet.getEmployeeId() + "|" + _species.getSpeciesId();
    }
}