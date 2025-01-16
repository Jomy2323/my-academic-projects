package hva.app.search;

import hva.Hotel;
import hva.Vet;
import hva.VaccinationEvent;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.app.exceptions.UnknownVeterinarianKeyException;
import hva.exceptions.UnknownVeterinarianException;
import hva.app.search.SearchStrategy;
import hva.exceptions.SearchException;


import java.util.List;
import java.util.stream.Collectors;

class DoShowMedicalActsByVeterinarian extends Command<Hotel> {

    DoShowMedicalActsByVeterinarian(Hotel receiver) {
        super(Label.MEDICAL_ACTS_BY_VET, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try {
            Form request = new Form();
            request.addStringField("vetKey", 
             hva.app.employee.Prompt.employeeKey());
            request.parse();

            String vetKey = request.stringField("vetKey");
            SearchStrategy searchStrategy = new SearchStrategy() {
                @Override
                public List<String> execute() throws
                 UnknownVeterinarianException{
                    Vet vet = _receiver.getHotelVet(vetKey);
                    return vet.getVaccinationHistory()
                                .stream()
                                .map(VaccinationEvent::toString)
                                .collect(Collectors.toList());
                }
            };

            List<String> medicalActs = searchStrategy.execute();
            _display.popup(medicalActs);
        }
        catch(UnknownVeterinarianException e){
            throw new UnknownVeterinarianKeyException(e.getKey());
        }
        catch(SearchException e){
            e.printStackTrace();
        }
    }

}
