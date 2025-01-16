package hva.app.search;

import hva.Hotel;
import hva.VaccinationEvent;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.search.SearchStrategy;
import hva.exceptions.SearchException;
import java.util.List;
import java.util.stream.Collectors;

class DoShowWrongVaccinations extends Command<Hotel> {

    DoShowWrongVaccinations(Hotel receiver) {
        super(Label.WRONG_VACCINATIONS, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{
            SearchStrategy searchStrategy = new SearchStrategy() {
                @Override
                public List<String> execute(){
                    return _receiver.getHotelVaccinationHistory()
                                    .stream()
                                    .filter(vaccination ->!vaccination
                                    .getEffect().equals("NORMAL"))
                                    .map(VaccinationEvent::toString)
                                    .collect(Collectors.toList());
                }
            };

            List<String> wrongVaccinations = searchStrategy.execute();
            _display.popup(wrongVaccinations);
        }
        catch (SearchException e){
            e.printStackTrace();
        }
    }

}
