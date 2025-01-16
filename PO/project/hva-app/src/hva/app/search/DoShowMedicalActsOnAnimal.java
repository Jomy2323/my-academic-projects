package hva.app.search;

import hva.Hotel;
import hva.Animal;
import hva.VaccinationEvent;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.exceptions.UnknownAnimalException;
import hva.app.search.SearchStrategy;
import hva.exceptions.SearchException;


import java.util.List;
import java.util.stream.Collectors;

class DoShowMedicalActsOnAnimal extends Command<Hotel> {

    DoShowMedicalActsOnAnimal(Hotel receiver) {
        super(Label.MEDICAL_ACTS_ON_ANIMAL, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("animalKey", 
             hva.app.animal.Prompt.animalKey());
            request.parse();

            String animalKey = request.stringField("animalKey");
            SearchStrategy searchStrategy = new SearchStrategy() {
                @Override
                public List<String> execute() throws
                 UnknownAnimalException{
                    Animal animal = _receiver.getHotelAnimal
                     (animalKey);
                    return animal.getAnimalVaccinationHistory()
                                    .stream()
                                    .map(VaccinationEvent::toString)
                                    .collect(Collectors.toList());   
                }
            };  
            List<String> medicalActs = searchStrategy.execute();
            _display.popup(medicalActs);
        }
        catch (UnknownAnimalException e) {
            throw new UnknownAnimalKeyException(e.getKey());
        }
        catch (SearchException e){
            e.printStackTrace();
        }
    }

}
