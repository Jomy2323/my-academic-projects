package hva.app.animal;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.DuplicateAnimalException;
import hva.exceptions.UnknownHabitatException;
import hva.exceptions.DuplicateSpeciesException;
import hva.app.exceptions.DuplicateAnimalKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;

class DoRegisterAnimal extends Command<Hotel> {

    DoRegisterAnimal(Hotel receiver) {
        super(Label.REGISTER_ANIMAL, receiver);
        
    }

    @Override
    protected final void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("animalKey", Prompt.animalKey());
            request.addStringField("animalName", Prompt.animalName());
            request.addStringField("speciesKey", Prompt.speciesKey());
            request.addStringField("habitatKey",
             hva.app.habitat.Prompt.habitatKey());
            request.parse();
    
            String speciesName = null;  
            boolean isNewSpecies = false;
            if (!_receiver.speciesExists(
                request.stringField("speciesKey"))) {
                Form speciesNameRequest = new Form();
                speciesNameRequest.addStringField("speciesName",
                 Prompt.speciesName());
                speciesNameRequest.parse();
    
                speciesName = 
                speciesNameRequest.stringField("speciesName");
                isNewSpecies = true; 
            }

            String animalKey = request.stringField("animalKey");
            String animalName = request.stringField("animalName");
            String speciesKey = request.stringField("speciesKey");
            String habitatKey = request.stringField("habitatKey");
    
            _receiver.registerAnimal(new String[] {null, animalKey,
                 animalName, speciesKey, habitatKey});
    
            if (isNewSpecies) {
                _receiver.registerSpecies(new String[] {null,
                     speciesKey, speciesName, animalKey, habitatKey,
                      animalName});
            }
    
        } catch (DuplicateAnimalException e) {
            throw new DuplicateAnimalKeyException(e.getKey());
        } catch (UnknownHabitatException e) {
            throw new UnknownHabitatKeyException(e.getKey());
        } catch (DuplicateSpeciesException e) {
            e.printStackTrace();
        }
    }
}