package hva;

import java.io.Serial;
import java.io.Serializable;
import java.util.Map;
import java.util.TreeMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Collections;

import hva.satisfaction.HandlerSatisfactionCalculator;
import hva.satisfaction.SatisfactionCalculator;

public class Handler extends Employee implements Serializable{

    /** handler's responsible habitats */
    private Map<String, Habitat> _handlerResponsibleHabitats;

    /**
     * Constructor
     * @param id the handler's id
     * @param name the handler's name
     */
    public Handler(String id, String name){
        super(id, name);
        _handlerResponsibleHabitats = new TreeMap<String, Habitat>(
            new CaseInsensitiveComparator());

        setSatisfactionCalculator(new
         HandlerSatisfactionCalculator(this));
    }

    //getter methods
    public Map<String, Habitat> getResponsabilityKey() {
        return _handlerResponsibleHabitats;
    }

    /**
     * Add a habitat to the handler's responsability
     * @param habitatId the habitat's id
     * @param habitat the habitat
     */
    public void addHandlerHabitat(String habitatId, Habitat habitat){
        _handlerResponsibleHabitats.put(habitatId, habitat);
    }

    /**
     * Remove a habitat from the handler's responsability
     * @param habitatId the habitat's id
     */
    public void removeHandlerHabitat(String habitatId){
        _handlerResponsibleHabitats.remove(habitatId);
    }

    @Override
    public String toString() {
        if (_handlerResponsibleHabitats.isEmpty()){
            return "TRT|" + super.toString();
        } else {
            List<String> sortedKeys =
             new ArrayList<>(_handlerResponsibleHabitats.keySet()); 
            String result = String.join(",", sortedKeys);
            return "TRT|" + super.toString() + "|" + result;
        }
    }
}