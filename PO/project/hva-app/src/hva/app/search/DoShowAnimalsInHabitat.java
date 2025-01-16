package hva.app.search;

import hva.Hotel;
import hva.Habitat;
import hva.Animal;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownHabitatKeyException;
import hva.exceptions.UnknownHabitatException;
import hva.app.search.SearchStrategy;
import hva.exceptions.SearchException;

import java.util.List;
import java.util.stream.Collectors;

class DoShowAnimalsInHabitat extends Command<Hotel> {

    DoShowAnimalsInHabitat(Hotel receiver) {
        super(Label.ANIMALS_IN_HABITAT, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("habitatKey", hva.app.habitat
             .Prompt.habitatKey());
            request.parse();
            String habitatKey = request.stringField("habitatKey");

            SearchStrategy searchStrategy = new SearchStrategy() {
                @Override
                public List<String> execute() throws
                 UnknownHabitatException {
                    Habitat habitat = _receiver
                     .getHotelHabitat(habitatKey);
                    return habitat.getHabitatAnimals().values()
                            .stream()
                            .map(Animal::toString)
                            .collect(Collectors.toList());
                }
            };
            List<String> animalsInHabitat = searchStrategy.execute();
            _display.popup(animalsInHabitat);
        }
        catch (UnknownHabitatException e) {
            throw new UnknownHabitatKeyException(e.getKey());
        }
        catch (SearchException e) {
            e.printStackTrace();
        }
    }
}
