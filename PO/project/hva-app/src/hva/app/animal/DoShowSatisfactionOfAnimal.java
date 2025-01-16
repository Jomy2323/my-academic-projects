package hva.app.animal;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import hva.app.exceptions.UnknownAnimalKeyException;
import hva.exceptions.UnknownAnimalException;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowSatisfactionOfAnimal extends Command<Hotel> {

    DoShowSatisfactionOfAnimal(Hotel receiver) {
        super(Label.SHOW_SATISFACTION_OF_ANIMAL, receiver);
    }

    @Override
    protected final void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("animalKey", Prompt.animalKey());
            request.parse();

            String animalKey = request.stringField("animalKey");
            _display.popup(_receiver.calculateAnimalSatisfaction
             (animalKey));
        }
        catch(UnknownAnimalException e){
            throw new UnknownAnimalKeyException(e.getKey());
        }
    }    

}
