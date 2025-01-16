package hva.app.animal;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownAnimalException;
import hva.exceptions.UnknownHabitatException;

class DoTransferToHabitat extends Command<Hotel> {

    DoTransferToHabitat(Hotel hotel) {
        super(Label.TRANSFER_ANIMAL_TO_HABITAT, hotel);
    }

    @Override
    protected final void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("animalKey", Prompt.animalKey());
            request.addStringField("habitatKey",
             hva.app.habitat.Prompt.habitatKey());
            request.parse();

            String animalKey = request.stringField("animalKey");
            String habitatKey = request.stringField("habitatKey");

            _receiver.transferAnimalToHabitat(animalKey, habitatKey);
        }
        catch (UnknownAnimalException e) {
            throw new UnknownAnimalKeyException(e.getKey());
        }
        catch (UnknownHabitatException e) {
            throw new UnknownHabitatKeyException(e.getKey());
        }
    }

}
