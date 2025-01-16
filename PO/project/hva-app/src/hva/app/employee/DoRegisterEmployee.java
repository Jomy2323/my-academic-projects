package hva.app.employee;

import hva.Hotel;
import pt.tecnico.uilib.forms.Form;
import pt.tecnico.uilib.menus.Command;
import pt.tecnico.uilib.menus.CommandException;
import hva.exceptions.DuplicateEmployeeException;
import hva.app.exceptions.DuplicateEmployeeKeyException;

class DoRegisterEmployee extends Command<Hotel> {

    DoRegisterEmployee(Hotel receiver) {
        super(Label.REGISTER_EMPLOYEE, receiver);
    }

    @Override
    protected void execute() throws CommandException {
        try{

            Form request = new Form();
            request.addStringField("employeeKey", Prompt.employeeKey
             ());
            request.addStringField("employeeName", Prompt.employeeName
             ());
            request.parse();

            String type;
            do{
                type = Form.requestString(Prompt.employeeType());
            } while (!type.equals("VET") && !type.equals("TRT"));

            type = switch(type){
                case "VET" -> "VETERINÁRIO";
                case "TRT" -> "TRATADOR";
                default ->"";
            };

            String employeeKey = request.stringField("employeeKey");
            String employeeName = request.stringField("employeeName");
            _receiver.registerEmployee(new String[]{type, employeeKey,
             employeeName});
        }
        catch (DuplicateEmployeeException e){
            throw new DuplicateEmployeeKeyException(e.getKey());
        }
    }

}
