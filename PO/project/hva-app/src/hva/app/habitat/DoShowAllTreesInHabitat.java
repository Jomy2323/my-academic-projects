package hva.app.habitat;

import hva.Hotel;
import hva.Habitat;
import hva.exceptions.UnknownHabitatException;
import hva.app.exceptions.UnknownHabitatKeyException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowAllTreesInHabitat extends Command<Hotel> {

    DoShowAllTreesInHabitat(Hotel receiver) {
        super(Label.SHOW_TREES_IN_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("habitatKey", Prompt.habitatKey());
            request.parse();
            
            String habitatKey = request.stringField("habitatKey");
            _display.popup(_receiver.showTreesInHabitat(habitatKey));
        } catch(UnknownHabitatException e){
            throw new UnknownHabitatKeyException(e.getKey());
        }


    }

}
