package hva.app.employee;

import hva.Hotel;
import hva.exceptions.UnknownEmployeeException;
import hva.app.exceptions.UnknownEmployeeKeyException;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;

class DoShowSatisfactionOfEmployee extends Command<Hotel> {

    DoShowSatisfactionOfEmployee(Hotel receiver) {
        super(Label.SHOW_SATISFACTION_OF_EMPLOYEE, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{
            Form request = new Form();
            request.addStringField("employeeKey", Prompt.employeeKey());
            request.parse();

            String employeeKey = request.stringField("employeeKey");
            _display.popup(_receiver.calculateEmployeeSatisfaction
            (employeeKey));
        }
        catch(UnknownEmployeeException e){
            throw new UnknownEmployeeKeyException(e.getKey());
        }
    }

}
